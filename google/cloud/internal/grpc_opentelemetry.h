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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPENTELEMETRY_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPENTELEMETRY_H

#include "google/cloud/completion_queue.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#include "google/cloud/internal/grpc_request_metadata.h"
#include "google/cloud/internal/opentelemetry.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/experimental_semantic_conventions.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer_provider.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include <chrono>
#include <functional>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

/**
 * Make a span that propagates its context by setting gRPC metadata.
 *
 * In order to get visibility into anything happening on the server side, we
 * need to associate the requests with the spans that we export. We accomplish
 * this by copying the span's attributes into the grpc::ClientContext as
 * metadata.
 */
opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpanGrpc(
    opentelemetry::nostd::string_view service,
    opentelemetry::nostd::string_view method);

template <typename T>
T CaptureReturn(
    grpc::ClientContext& context,
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
    T value) {
  // TODO(dbolduc): "grpc.peer" is not a real key. We need to split it into
  //                AttrNetPeerIp and AttrNetPeerPort.
  // Capture at least one field from the ClientContext as a span attribute.
  span->SetAttribute("grpc.peer", context.peer());
  // TODO(dbolduc): do we need to grab the metadata, so we can Extract a remote
  // span context? I don't think we need to. That is not really how our clients
  // work. Maybe a pubsub subscription works like that?
  /*
  auto md = GetRequestMetadataFromContext(context);
  for (auto const& kv : md) {
    std::cout << kv.first << ": " << kv.second << std::endl;
  }
  std::cout << std::endl;
  */

  return CaptureReturn(std::move(span), std::move(value));
}

// We need to extract information from the grpc::ClientContext that is set
// during the call, such as the peer it connects to.
//
// For sync calls (handled by `RetryLoopImpl`), we create a ClientContext on the
// stack, which is valid until an iteration of the loop is done. We know it will
// exist by the end of a Stub call.
//
// For async calls, we allocate a unique_ptr to a ClientContext. The underlying
// object will be destroyed when it is no longer used. We extend its lifetime so
// that we can extract its goodies by using a shared_ptr.
template <typename T>
T CaptureReturn(
    std::shared_ptr<grpc::ClientContext> const& context,
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
    T value) {
  // Capture at least one field from the ClientContext as a span attribute.
  span->SetAttribute("grpc.peer", context->peer());
  return CaptureReturn(std::move(span), std::move(value));
}

void InjectTraceContext(grpc::ClientContext& context,
                        Options const& options = CurrentOptions());

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

/**
 * Returns a traced timer, if OpenTelemetry tracing is enabled.
 */
template <typename Rep, typename Period>
future<StatusOr<std::chrono::system_clock::time_point>> TracedAsyncBackoff(
    CompletionQueue& cq, std::chrono::duration<Rep, Period> duration) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  if (TracingEnabled()) {
    auto span = MakeSpan("Async Backoff");
    return internal::CaptureReturn(span, cq.MakeRelativeTimer(duration));
  }
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
  return cq.MakeRelativeTimer(duration);
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPENTELEMETRY_H
