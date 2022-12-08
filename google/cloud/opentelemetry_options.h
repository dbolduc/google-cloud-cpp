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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_OPTIONS_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_OPTIONS_H

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include "google/cloud/options.h"
#include "google/cloud/version.h"
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/tracer_provider.h>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace experimental {

// TODO(dbolduc): need to make a judgment call on the exposed API. (I think). Or
// have an opinion that will inform the design.
/**
 * Enable tracing with [OpenTelemetry]
 *
 * Trace API calls to and from the remote server. The client library will create
 * spans for interesting work that it performs. These spans can be exported to
 * the trace visualizer backend of your choice.
 *
 * @note Tracing has not been implemented by all libraries.
 *
 * @note The library must be compiled with OpenTelemetry in order for this
 * option to take effect. The option must be supplied to `Make*Connection()` in
 * order to take effect.
 *
 * @see TODO : EXAMPLE to learn how to compile with OpenTelemetry.
 *
 * @see TODO : EXAMPLE to learn how to export spans to Cloud Trace.
 *
 * TODO : Naming... I don't like how "TracingComponentsOption" is named...
 * is it tracing... or logging....
 *
 * [opentelemetry]: https://opentelemetry.io/docs/instrumentation/cpp/
 */
struct OpenTelemetryTracingOption {
  using Type = bool;
};

// TODO(dbolduc): this was dashpole's suggestion. It makes sense. It can enable
// tracing per call / client. (otherwise we would probably use the global one)
//
// Such as the Cloud Trace Exporter (`GcpExporter`)
struct OpenTelemetryTracerProviderOption {
  using Type =
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>;
};

// TODO(dbolduc): this was dashpole's suggestion. It makes sense. It can enable
// a different propagator per call / client. (otherwise we would probably use
// the global one)
//
// Such as the Cloud Trace Propagator (`CloudTraceContext`)
struct OpenTelemetryTextMapPropagatorOption {
  using Type = opentelemetry::nostd::shared_ptr<
      opentelemetry::context::propagation::TextMapPropagator>;
};

using OpenTelemetryOptions =
    OptionList<OpenTelemetryTracingOption, OpenTelemetryTracerProviderOption,
               OpenTelemetryTextMapPropagatorOption>;

}  // namespace experimental
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_OPTIONS_H
