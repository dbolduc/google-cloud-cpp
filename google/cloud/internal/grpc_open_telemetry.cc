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

#include "google/cloud/internal/grpc_open_telemetry.h"
#include <grpcpp/client_context.h>
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/context/propagation/global_propagator.h>
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
namespace {

class GrpcClientCarrier
    : public opentelemetry::context::propagation::TextMapCarrier {
 public:
  explicit GrpcClientCarrier(grpc::ClientContext& context)
      : context_(context) {}

  opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view) const noexcept override {
    return "";
  }

  void Set(opentelemetry::nostd::string_view key,
           opentelemetry::nostd::string_view value) noexcept override {
    // Forward the trace context over the network. We can potentially associate
    // this call with traces that Google collects on the server-side.
    context_.AddMetadata(key.data(), value.data());
  }

  grpc::ClientContext& context_;
};

}  // namespace

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    grpc::ClientContext&, opentelemetry::nostd::string_view name) {
  // Start a span, setting at least one attribute specific to grpc
  auto span = GetTracer()->StartSpan(
      name, {{OTEL_GET_TRACE_ATTR(AttrRpcSystem), "grpc"}},
      {.kind = opentelemetry::trace::SpanKind::kClient});
  return span;
}

void InjectSpanContext(grpc::ClientContext& context) {
  auto current = opentelemetry::context::RuntimeContext::GetCurrent();
  GrpcClientCarrier carrier(context);
  auto prop = opentelemetry::context::propagation::GlobalTextMapPropagator::
      GetGlobalPropagator();
  prop->Inject(carrier, current);
}

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
