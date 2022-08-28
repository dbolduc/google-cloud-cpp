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

#include "google/cloud/internal/scoped_span.h"
#include <opentelemetry/trace/provider.h>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer() {
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer("gcloud-cpp", google::cloud::version_string());
}

void CaptureStatusDetails(opentelemetry::trace::Span& span,
                          Status const& status, bool end) {
  auto constexpr kMaxAttributeLen = 128;
  if (status.ok()) {
    span.SetAttribute("gcloud.status_code", "OK");
    if (end) span.End();
    return;
  }
  span.SetAttribute("gcloud.status_code", StatusCodeToString(status.code()));
  span.SetAttribute("gcloud.status_message",
                    status.message().substr(0, kMaxAttributeLen));
  if (!status.error_info().reason().empty()) {
    span.SetAttribute("gcloud.status_error_info_reason",
                      status.error_info().reason().substr(0, kMaxAttributeLen));
  }
  if (!status.error_info().domain().empty()) {
    span.SetAttribute("gcloud.status_error_info_domain",
                      status.error_info().domain().substr(0, kMaxAttributeLen));
  }
  for (auto const& kv : status.error_info().metadata()) {
    span.SetAttribute("gcloud.status_error_info_" + kv.first,
                      kv.second.substr(0, kMaxAttributeLen));
  }
  if (end) span.End();
}

ScopedSpan ScopedSpan::StartScopedSpan(
    opentelemetry::nostd::string_view name,
    std::initializer_list<std::pair<opentelemetry::nostd::string_view,
                                    opentelemetry::common::AttributeValue>>
        attributes,
    opentelemetry::trace::StartSpanOptions const& options) {
  auto span = GetTracer()->StartSpan(name, std::move(attributes), options);
  auto scope = opentelemetry::trace::Tracer::WithActiveSpan(span);
  return ScopedSpan(std::move(span), std::move(scope));
}

void ScopedSpan::CaptureStatusDetails(google::cloud::Status const& status) {
  return ::google::cloud::internal::CaptureStatusDetails(*span_, status, false);
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
