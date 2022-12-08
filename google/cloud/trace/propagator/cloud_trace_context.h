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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TRACE_PROPAGATOR_CLOUD_TRACE_CONTEXT_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TRACE_PROPAGATOR_CLOUD_TRACE_CONTEXT_H

// TODO(dbolduc) : I think this belongs upstream in:
// https://github.com/GoogleCloudPlatform/opentelemetry-operations-cpp
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include "google/cloud/version.h"
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/context.h>
#include <opentelemetry/trace/default_span.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>

// TODO(dbolduc) : I don't know what namespace we should be using. Probably not
// ours.
namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace experimental {

/**
 * A context propagator, specifically for Google Cloud.
 *
 * @see https://cloud.google.com/trace/docs/setup#force-trace for the
 * implementation specification.
 */
class CloudTraceContext
    : public opentelemetry::context::propagation::TextMapPropagator {
 public:
  explicit CloudTraceContext() = default;

  // Returns the context that is stored in the carrier with the TextMapCarrier
  // as extractor.
  opentelemetry::context::Context Extract(
      opentelemetry::context::propagation::TextMapCarrier const&,
      opentelemetry::context::Context& context) noexcept override {
    // TODO(dbolduc): Extract the context. We can copy the jaeger propagator.
    auto span_context = opentelemetry::trace::SpanContext::GetInvalid();
    auto span = opentelemetry::trace::DefaultSpan::GetInvalid();
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> sp{&span};
    return opentelemetry::trace::SetSpan(context, sp);
  }

  // Sets the context for carrier with self defined rules.
  void Inject(opentelemetry::context::propagation::TextMapCarrier& carrier,
              opentelemetry::context::Context const& context) noexcept override;

  // Gets the fields set in the carrier by the `inject` method
  bool Fields(opentelemetry::nostd::function_ref<
              bool(opentelemetry::nostd::string_view)>
                  callback) const noexcept override {
    return callback("x-cloud-trace-context");
  }
};

}  // namespace experimental
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TRACE_PROPAGATOR_CLOUD_TRACE_CONTEXT_H
