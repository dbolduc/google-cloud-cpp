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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPEN_TELEMETRY_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPEN_TELEMETRY_H

#include "google/cloud/internal/open_telemetry.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/experimental_semantic_conventions.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer_provider.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <grpcpp/grpcpp.h>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

/**
 * Make a span that propagates its context by setting gRPC metadata.
 *
 * In order to get visibility into anything happening on the server side, we
 * need to associate the requests with the spans that we export. We accomplish
 * this by copying the span's attributes into the grpc::ClientContext's as
 * metadata.
 */
opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    grpc::ClientContext& context, opentelemetry::nostd::string_view name);

template <typename T>
T CaptureReturn(grpc::ClientContext& context, opentelemetry::trace::Span& span,
                T value, bool end) {
  // TODO(dbolduc): "grpc.peer" is not a real key. We need to split it into
  //                AttrNetPeerIp and AttrNetPeerPort.
  // Capture at least one field from the ClientContext as a span attribute.
  span.SetAttribute("grpc.peer", context.peer());
  return CaptureReturn(span, std::move(value), end);
}

void InjectSpanContext(grpc::ClientContext& context);

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPEN_TELEMETRY_H
