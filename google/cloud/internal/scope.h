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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPE_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPE_H

#include "google/cloud/options.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/context.h>
#include <opentelemetry/trace/default_span.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/tracer.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <chrono>
#include <functional>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

using OptionsScope = OptionsSpan;

// TODO(dbolduc): I think these two classes have A LOT of potential.
//
// For OT tracing, we need to configure the grpc::ClientContext using the
// current span. In general, we configure the grpc::ClientContext using the
// CurrentOptions (e.g. to apply GrpcSetupOption). I think there is an
// opportunity for consolidation here!
//
// It would be great if we just call:
// `Inject(grpc::ClientContext&, CurrentDarren)` on the way in and
// `Extract(grpc::ClientContext&, CurrentDarren)` on the way out.
// (Extract would hopefully replace CaptureReturn)
//
// It would be amazing if there is a generic form that works for REST too.
//
// Basically I think we are going to need to do #9454 for OT tracing anyway.
// https://github.com/googleapis/google-cloud-cpp/issues/9454
//
struct CurrentDarren {
  Options options = CurrentOptions();
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> trace_span =
      opentelemetry::trace::Tracer::GetCurrentSpan();
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
};

// For propagating context across `AsyncGrpcOperation`s
class DarrenScope {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
 public:
  explicit DarrenScope(CurrentDarren darren)
      : options_scope_(std::move(darren.options)),
        trace_scope_(std::move(darren.trace_span)) {}

 private:
  OptionsScope options_scope_;
  opentelemetry::trace::Scope trace_scope_;

#else
 public:
  explicit DarrenScope(CurrentDarren darren)
      : options_scope_(std::move(darren.options)) {}

 private:
  OptionsScope options_scope_;

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
};

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_SCOPE_H
