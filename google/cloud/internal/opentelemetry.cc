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

#include "google/cloud/internal/opentelemetry.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#include "google/cloud/opentelemetry_options.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include <opentelemetry/context/context.h>
#include <opentelemetry/trace/context.h>
#include <opentelemetry/trace/noop.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/span_metadata.h>
#include <opentelemetry/trace/span_startoptions.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer(
    Options const& options) {
  auto provider =
      options.has<experimental::OpenTelemetryTracerProviderOption>()
          ? options.get<experimental::OpenTelemetryTracerProviderOption>()
          : opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer("gcloud-cpp", version_string());
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    opentelemetry::nostd::string_view name) {
  return GetTracer()->StartSpan(
      name, {.kind = opentelemetry::trace::SpanKind::kClient});
}

void CaptureStatusDetails(opentelemetry::trace::Span& span,
                          Status const& status) {
  auto constexpr kMaxAttributeLen = 128;
  if (status.ok()) {
    span.SetStatus(opentelemetry::trace::StatusCode::kOk);
    span.SetAttribute("gcloud.status_code", "OK");
    span.End();
    return;
  }
  span.SetStatus(opentelemetry::trace::StatusCode::kError);
  span.SetAttribute("gcloud.status_code", StatusCodeToString(status.code()));
  span.SetAttribute("gcloud.status_message",
                    status.message().substr(0, kMaxAttributeLen));
  span.End();
}

Status CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    Status status) {
  CaptureStatusDetails(*span, status);
  return status;
}

future<Status> CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    future<Status> fut) {
  return fut.then([=](auto f) {
    auto status = f.get();
    CaptureStatusDetails(*span, status);
    return status;
  });
}

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

std::function<void(std::chrono::milliseconds)> MakeTracingSleeper(
    std::function<void(std::chrono::milliseconds)> const& sleeper) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  if (TracingEnabled()) {
    return [=](std::chrono::milliseconds p) {
      auto span = MakeSpan("Backoff");
      sleeper(p);
      span->End();
    };
  }
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  return sleeper;
}

// NOLINTNEXTLINE(misc-unused-value-param)
void AddSpanAttribute(std::string const& key, std::string const& value) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  if (TracingEnabled()) {
    auto span = opentelemetry::trace::Tracer::GetCurrentSpan();
    span->SetAttribute(std::move(key), std::move(value));
  }
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
}

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
bool TracingEnabled(Options const& options) {
  // Use a bool option, which we define.
  return options.get<experimental::OpenTelemetryTracingOption>();

  // Inspect the global tracer provider.
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  return nullptr == dynamic_cast<opentelemetry::trace::NoopTracerProvider*>(
                        provider.get());
}
#else
bool TracingEnabled(Options const&) { return false; };
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
