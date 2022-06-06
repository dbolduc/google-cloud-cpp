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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_DARREN_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_DARREN_H

#include "google/cloud/internal/async_streaming_read_rpc.h"
#include "absl/types/variant.h"
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace bigtable_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

/**
 * A wrapper for asynchronous gRPC streaming read RPCs.
 *
 * Performs one loop of a streaming read.
 *
 * This is a replacement for CQ::MakeStreamingReadRpc()
 *
 * TODO : Hooks for OnRead, OnFinish
 */
template <
    typename Response, typename OnReadHandler, typename OnFinishHandler,
    // TODO : static_assert seems more appropriate. It needn't compile.
    typename std::enable_if<
        google::cloud::internal::is_invocable<OnReadHandler, Response>::value,
        int>::type OnReadIsInvocableWithResponse = 0,
    typename std::enable_if<
        std::is_same<future<bool>, google::cloud::internal::invoke_result_t<
                                       OnReadHandler, Response>>::value,
        int>::type OnReadReturnsFutureBool = 0,
    typename std::enable_if<
        google::cloud::internal::is_invocable<OnFinishHandler, Status>::value,
        int>::type OnFinishIsInvocableWithStatus = 0>
class Darren : public std::enable_shared_from_this<
                   Darren<Response, OnReadHandler, OnFinishHandler>> {
 public:
  explicit Darren(
      std::unique_ptr<internal::AsyncStreamingReadRpc<Response>> stream,
      OnReadHandler&& on_read, OnFinishHandler&& on_finish)
      : stream_(std::move(stream)),
        on_read_(std::move(on_read)),
        on_finish_(std::move(on_finish)) {}

  void Start() {
    std::cout << "AsyncStreamLoop::Start()" << std::endl;
    auto self = this->shared_from_this();
    auto start = stream_->Start();
    start.then([self](future<bool> f) {
      // Start was unsuccessful, finish stream.
      if (!f.get()) return self->Finish();
      // Start was successful, start reading.
      self->Read();
    });
  }

 private:
  void Read() {
    std::cout << "AsyncStreamLoop::Read()" << std::endl;
    auto self = this->shared_from_this();
    auto read = stream_->Read();
    read.then([self](future<absl::optional<Response>> f) {
      auto r = f.get();
      // Read did not yield a response, finish stream.
      if (!r.has_value()) return self->Finish();
      // Read yielded a response, return it.
      std::cout << "Received resp: " << r->DebugString() << std::endl;
      self->on_read_(*std::move(r)).then([self](future<bool> keep_reading) {
          // TODO : this isn't right. we cannot just finish. we must discard.
        if (keep_reading.get()) return self->Read();
        self->stream_->Cancel();
        self->Discard();
      });
    });
  }

  void Discard() {
    std::cout << "AsyncStreamLoop::Discard()" << std::endl;
    auto self = this->shared_from_this();
    auto read = stream_->Read();
    read.then([self](future<absl::optional<Response>> f) {
      auto r = f.get();
      // Read did not yield a response, finish stream.
      if (!r.has_value()) return self->Finish();
      // Read yielded a response, keep discarding.
      self->Discard();
    });
  }

  void Finish() {
    std::cout << "AsyncStreamLoop::Finish()" << std::endl;
    auto self = this->shared_from_this();
    auto finish = stream_->Finish();
    finish.then([self](future<Status> f) { self->on_finish_(f.get()); });
  }

  std::unique_ptr<internal::AsyncStreamingReadRpc<Response>> stream_;
  typename std::decay<OnReadHandler>::type on_read_;
  typename std::decay<OnFinishHandler>::type on_finish_;
};

  template <
      typename Response, typename OnReadHandler, typename OnFinishHandler,
      // TODO : static_assert seems more appropriate. It needn't compile.
      typename std::enable_if<
          google::cloud::internal::is_invocable<OnReadHandler, Response>::value,
          int>::type OnReadIsInvocableWithResponse = 0,
      typename std::enable_if<
          std::is_same<future<bool>, google::cloud::internal::invoke_result_t<
                                         OnReadHandler, Response>>::value,
          int>::type OnReadReturnsFutureBool = 0,
      typename std::enable_if<
          google::cloud::internal::is_invocable<OnFinishHandler, Status>::value,
          int>::type OnFinishIsInvocableWithStatus = 0>
  void AsyncStreamLoop(
      std::unique_ptr<internal::AsyncStreamingReadRpc<Response>> stream,
      OnReadHandler&& on_read, OnFinishHandler&& on_finish) {
    // TODO :
    std::cout << "Constructing AsyncStreamLoop..." << std::endl;
    auto darren =
        std::make_shared<Darren<Response, OnReadHandler, OnFinishHandler>>(
            std::move(stream), std::forward<OnReadHandler>(on_read),
            std::forward<OnFinishHandler>(on_finish));
    darren->Start();
  }

  GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_DARREN_H
