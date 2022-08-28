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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPED_SPAN_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPED_SPAN_H

#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include "google/cloud/version.h"
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer();

void CaptureStatusDetails(opentelemetry::trace::Span& span,
                          Status const& status, bool end);

template <typename T>
StatusOr<T> CaptureReturn(opentelemetry::trace::Span& span, StatusOr<T> value,
                          bool end) {
  CaptureStatusDetails(span, value.status(), end);
  return value;
}

inline Status CaptureReturn(opentelemetry::trace::Span& span, Status value,
                            bool end) {
  CaptureStatusDetails(span, value, end);
  return value;
}

class ScopedSpan {
 public:
  static ScopedSpan StartScopedSpan(
      opentelemetry::nostd::string_view name,
      std::initializer_list<std::pair<opentelemetry::nostd::string_view,
                                      opentelemetry::common::AttributeValue>>
          attributes,
      opentelemetry::trace::StartSpanOptions const& options = {});

  ScopedSpan(ScopedSpan&&) noexcept = default;
  ScopedSpan& operator=(ScopedSpan&&) noexcept = default;
  ~ScopedSpan() { span_->End(); }

  template <typename T>
  StatusOr<T> CaptureReturn(StatusOr<T> value) {
    CaptureStatusDetails(value.status());
    return value;
  }

  Status CaptureReturn(Status value) {
    CaptureStatusDetails(value);
    return value;
  }

  void SetAttribute(opentelemetry::nostd::string_view key,
                    opentelemetry::nostd::string_view value) {
    span_->SetAttribute(key, value);
  }
  void SetAttribute(opentelemetry::nostd::string_view key, std::int32_t value) {
    span_->SetAttribute(key, value);
  }

 private:
  explicit ScopedSpan(
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
      opentelemetry::trace::Scope scope)
      : span_(std::move(span)), scope_(std::move(scope)) {}

  void CaptureStatusDetails(Status const& status);

  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span_;
  opentelemetry::trace::Scope scope_;
};

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPED_SPAN_H
