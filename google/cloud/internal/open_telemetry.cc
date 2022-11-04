// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/internal/open_telemetry.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/context/context.h>
#include <opentelemetry/trace/context.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/span_metadata.h>
#include <opentelemetry/trace/span_startoptions.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer() {
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer("gcloud-cpp", google::cloud::version_string());
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    opentelemetry::nostd::string_view name) {
  return GetTracer()->StartSpan(
      name, {.kind = opentelemetry::trace::SpanKind::kClient});
}

void CaptureStatusDetails(opentelemetry::trace::Span& span,
                          Status const& status, bool end) {
  auto constexpr kMaxAttributeLen = 128;
  if (status.ok()) {
    span.SetStatus(opentelemetry::trace::StatusCode::kOk);
    span.SetAttribute("gcloud.status_code", "OK");
    if (end) span.End();
    return;
  }
  span.SetStatus(opentelemetry::trace::StatusCode::kError);
  span.SetAttribute("gcloud.status_code", StatusCodeToString(status.code()));
  span.SetAttribute("gcloud.status_message",
                    status.message().substr(0, kMaxAttributeLen));
  if (end) span.End();
}

Status CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    Status status, bool end) {
  CaptureStatusDetails(*span, status, end);
  return status;
}

future<Status> CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    future<Status> fut, bool end) {
  return fut.then([=](auto f) {
    auto status = f.get();
    CaptureStatusDetails(*span, status, end);
    return status;
  });
}

void CloudTraceContext::Inject(
    opentelemetry::context::propagation::TextMapCarrier& carrier,
    opentelemetry::context::Context const& context) noexcept {
  using opentelemetry::nostd::span;
  using opentelemetry::trace::SpanId;
  using opentelemetry::trace::TraceId;

  auto span_context = opentelemetry::trace::GetSpan(context)->GetContext();
  if (!span_context.IsValid()) {
    return;
  }

  // Format: X-Cloud-Trace-Context: TRACE_ID/SPAN_ID;o=TRACE_TRUE
  // Where (annoyingly) SPAN_ID is in decimal, not hex.

  // First convert the span_id from a hex array -> std::uint64_t -> dec string
  std::array<char, 2 * SpanId::kSize + 1> span_id;
  span_context.span_id().ToLowerBase16(
      span<char, 2 * SpanId::kSize>(span_id.data(), 2 * SpanId::kSize));
  span_id.back() = '\0';
  char* end;
  auto span_id_dec = std::strtoull(span_id.data(), &end, 16);
  auto span_id_dec_str = std::to_string(span_id_dec);

  // Then write in the values.
  std::size_t const trace_id_length = 32;
  std::size_t const span_id_dec_length = span_id_dec_str.size();
  std::size_t const total_size = trace_id_length + span_id_dec_length + 6;
  char trace_identity[total_size];

  span_context.trace_id().ToLowerBase16(
      span<char, 2 * TraceId::kSize>{&trace_identity[0], trace_id_length});
  trace_identity[trace_id_length] = '/';
  strcpy(&trace_identity[trace_id_length + 1], span_id_dec_str.c_str());
  trace_identity[trace_id_length + span_id_dec_length + 1] = ';';
  trace_identity[trace_id_length + span_id_dec_length + 2] = 'o';
  trace_identity[trace_id_length + span_id_dec_length + 3] = '=';
  trace_identity[trace_id_length + span_id_dec_length + 4] =
      span_context.IsSampled() ? '1' : '0';
  // TODO(dbolduc): why must I terminate this, when the others do not?
  trace_identity[trace_id_length + span_id_dec_length + 5] = '\0';

  carrier.Set("x-cloud-trace-context",
              opentelemetry::nostd::string_view(trace_identity));
}

std::function<void(std::chrono::milliseconds)> MakeTracingSleeper(
    char const* location,
    std::function<void(std::chrono::milliseconds)> const& sleeper) {
  return [=](std::chrono::milliseconds p) {
    auto span = MakeSpan(absl::StrCat(location, "::backoff"));
    sleeper(p);
    span->End();
  };
}

#else

std::function<void(std::chrono::milliseconds)> MakeTracingSleeper(
    char const*,
    std::function<void(std::chrono::milliseconds)> const& sleeper) {
  return sleeper;
}

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

void AddSpanAttribute(std::string key, std::string value) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
  auto span = opentelemetry::trace::Tracer::GetCurrentSpan();
  span->SetAttribute(std::move(key), std::move(value));
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
