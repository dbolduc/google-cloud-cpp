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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_OPEN_TELEMETRY_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_OPEN_TELEMETRY_H

#include "google/cloud/status_or.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/span.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

// I am not recommending that we put this all in one file.
// This is just a proof of concept.

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer();

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    opentelemetry::nostd::string_view name);

void CaptureStatusDetails(opentelemetry::trace::Span& span,
                          Status const& status, bool end);

template <typename T>
StatusOr<T> CaptureReturn(opentelemetry::trace::Span& span, StatusOr<T> value,
                          bool end) {
  CaptureStatusDetails(span, value.status(), end);
  return value;
}

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
        //
std::function<void(std::chrono::milliseconds)> MakeTracingSleeper(
    char const* location,
    std::function<void(std::chrono::milliseconds)> const& sleeper);

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_OPEN_TELEMETRY_H
