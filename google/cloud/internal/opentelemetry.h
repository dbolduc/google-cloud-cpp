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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_OPENTELEMETRY_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_OPENTELEMETRY_H

#include "google/cloud/future.h"
#include "google/cloud/status_or.h"
#include "google/cloud/stream_range.h"
#include "google/cloud/version.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/context.h>
#include <opentelemetry/trace/default_span.h>
#include <opentelemetry/trace/scope.h>
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

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

/**
 * Returns a tracer for use with tracing.
 *
 * If no TracerProvider is present in `options`, the global TracerProvider is
 * used. This is only called if tracing is enabled in a client.
 */
opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer(
    Options const& options = CurrentOptions());

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> MakeSpan(
    opentelemetry::nostd::string_view name);

void CaptureStatusDetails(opentelemetry::trace::Span& span,
                          Status const& status);

Status CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    Status status);

future<Status> CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    future<Status> fut);

template <typename T>
StatusOr<T> CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    StatusOr<T> value) {
  CaptureStatusDetails(*span, value.status());
  return value;
}

template <typename T>
future<StatusOr<T>> CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    future<StatusOr<T>> fut) {
  return fut.then([=](auto f) {
    auto value = f.get();
    CaptureStatusDetails(*span, value.status());
    return value;
  });
}

template <typename T>
class TracedStreamRange {
 public:
  explicit TracedStreamRange(
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
      StreamRange<T> sr)
      : span_(std::move(span)),
        scope_(absl::make_unique<opentelemetry::trace::Scope>(span_)),
        sr_(std::move(sr)),
        it_(sr_.begin()) {}

  ~TracedStreamRange() {
    // It is ok not to iterate the full range. We still need to end our span.
    (void)End();
  }

  absl::variant<Status, T> Advance() {
    if (it_ == sr_.end()) return End();
    auto sor = *it_;
    it_++;
    if (!sor) return End(std::move(sor).status());
    return *sor;
  }

 private:
  Status End(Status s = Status()) {
    if (scope_ != nullptr) {
      s = CaptureReturn(span_, std::move(s));
      scope_.reset();
    }
    return s;
  }

  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span_;
  std::unique_ptr<opentelemetry::trace::Scope> scope_;
  StreamRange<T> sr_;
  typename StreamRange<T>::iterator it_;
};

template <typename T>
StreamRange<T> CaptureReturn(
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> const& span,
    StreamRange<T> sr) {
  // StreamRange is not copyable, but a shared ptr to one is.
  auto impl = std::make_shared<TracedStreamRange<T>>(span, std::move(sr));
  auto reader = [impl = std::move(impl)] { return impl->Advance(); };
  return internal::MakeStreamRange<T>(std::move(reader));
}

// TODO(dbolduc): this was for debugging. Delete whenever.
struct LoggingOTelScopedSpan {
  explicit LoggingOTelScopedSpan(std::string name)
      : name_(std::move(name)),
        span_(GetTracer()->StartSpan(name_)),
        scope_(absl::make_unique<opentelemetry::trace::Scope>(span_)) {
    std::cout << "Span created: " << name_ << std::endl;
  }

  ~LoggingOTelScopedSpan() {
    std::cout << "Span destroyed: " << name_ << std::endl;
  }

  std::string name_;
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span_;
  std::unique_ptr<opentelemetry::trace::Scope> scope_;
};

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

std::function<void(std::chrono::milliseconds)> MakeTracingSleeper(
    std::function<void(std::chrono::milliseconds)> const& sleeper);

void AddSpanAttribute(std::string const& key, std::string const& value);

bool TracingEnabled(Options const& options = CurrentOptions());

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_INTERNAL_OPENTELEMETRY_H
