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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_CALL_CONTEXT_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_CALL_CONTEXT_H

#include "google/cloud/options.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/context.h>
#include <opentelemetry/trace/default_span.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/tracer.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include <chrono>
#include <functional>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

using OptionsScope = OptionsSpan;

// TODO(dbolduc): I think these two classes have potential.
//
// For OTel tracing, we need to configure the grpc::ClientContext using the
// current span. In general, we configure the grpc::ClientContext using the
// CurrentOptions (e.g. to apply GrpcSetupOption). I think there is an
// opportunity for consolidation here!
//
// It would be great if we just call:
// `Inject(grpc::ClientContext&, CallContext)` on the way in and
// `Extract(grpc::ClientContext&, CallContext)` on the way out.
// (Extract would hopefully replace CaptureReturn)
//
// It would be amazing if there is a generic form that works for REST too.
//
// Basically I think we are going to need to do #9454 for OTel tracing anyway.
// https://github.com/googleapis/google-cloud-cpp/issues/9454
//
struct CallContext {
  Options options = CurrentOptions();
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> trace_span =
      opentelemetry::trace::Tracer::GetCurrentSpan();
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
};

// For propagating context across `AsyncGrpcOperation`s
class ScopedCallContext {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
 public:
  explicit ScopedCallContext(CallContext call_context)
      : options_scope_(std::move(call_context.options)),
        trace_scope_(std::move(call_context.trace_span)) {}

 private:
  OptionsScope options_scope_;
  opentelemetry::trace::Scope trace_scope_;

#else
 public:
  explicit ScopedCallContext(CallContext call_context)
      : options_scope_(std::move(call_context.options)) {}

 private:
  OptionsScope options_scope_;

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
};

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {
namespace alternative {

// would exist in google/cloud/internal/opentelemetry.h
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
using Span = opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>;
using ScopedSpan = opentelemetry::trace::Scope;
#else
using Span = std::nullptr_t;
using ScopedSpan = std::nullptr_t;
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

Span CurrentSpan() {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  opentelemetry::trace::Tracer::GetCurrentSpan();
#else
  return nullptr;
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
}

// would exist in google/cloud/internal/call_context.h
struct CallContext {
  Options options = CurrentOptions();
  Span span = CurrentSpan();
};

class ScopedCallContext {
 public:
  explicit ScopedCallContext(CallContext call_context)
      : options_(std::move(call_context.options)),
        span_(std::move(call_context.span)) {}

 private:
  using ScopedOptions = OptionsSpan;
  ScopedOptions options_;
  ScopedSpan span_;
};

}  // namespace alternative
}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_CALL_CONTEXT_H
