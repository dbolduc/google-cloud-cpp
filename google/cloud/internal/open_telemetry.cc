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

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
