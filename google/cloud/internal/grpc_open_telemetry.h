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

#include "google/cloud/completion_queue.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#include "google/cloud/internal/open_telemetry.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/experimental_semantic_conventions.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer_provider.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <chrono>
#include <functional>

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
 * this by copying the span's attributes into the grpc::ClientContext as
 * metadata.
 */
opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    grpc::ClientContext& context, opentelemetry::nostd::string_view name);

template <typename T>
T CaptureReturn(
    grpc::ClientContext& context,
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span, T value,
    bool end) {
  // TODO(dbolduc): "grpc.peer" is not a real key. We need to split it into
  //                AttrNetPeerIp and AttrNetPeerPort.
  // Capture at least one field from the ClientContext as a span attribute.
  span->SetAttribute("grpc.peer", context.peer());
  return CaptureReturn(std::move(span), std::move(value), end);
}

void InjectSpanContext(grpc::ClientContext& context);

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

/**
 * Returns a traced timer, if the library was built with OpenTelemetry.
 *
 * @warning We cannot call `.then()` inline on the future when it is created. If
 * we do that, the span is still active when the next async operation starts,
 * meaning the next operation will be a child of this one. We do not want that.
 *
 * TODO(dbolduc) : We might be able to enforce this constraint in the API. We
 * could accept a callback parameter:
 * `std::function<void(StatusOr<std::chrono::system_clock::time_point>)> cb`
 * and do the `.then()` ourselves. Maybe not though.
 */
template <typename Rep, typename Period>
future<StatusOr<std::chrono::system_clock::time_point>> TracedAsyncBackoff(
    std::string const& location, CompletionQueue& cq,
    std::chrono::duration<Rep, Period> duration) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
  auto span = MakeSpan(absl::StrCat(location, "::backoff"));
  // auto scope = internal::GetTracer()->WithActiveSpan(span);
  return internal::CaptureReturn(span, cq.MakeRelativeTimer(duration), true);
#else
  return cq.MakeRelativeTimer(duration);
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_GRPC_OPEN_TELEMETRY_H
