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

#include "google/cloud/future.h"
#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include "google/cloud/version.h"
#include <opentelemetry/nostd/string_view.h>
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

inline future<Status> CaptureReturn(opentelemetry::trace::Span& span, future<Status> fut,
                          bool end) {
  return fut.then([&span, end](auto f) {
    auto status = f.get();
    CaptureStatusDetails(span, status, end);
    return status;
  });
}

template <typename T>
future<StatusOr<T>> CaptureReturn(opentelemetry::trace::Span& span,
                                  future<StatusOr<T>> fut, bool end) {
  return fut.then([&span, end](auto f) {
    auto value = f.get();
    CaptureStatusDetails(span, value.status(), end);
    return value;
  });
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    opentelemetry::nostd::string_view name);

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPED_SPAN_H
