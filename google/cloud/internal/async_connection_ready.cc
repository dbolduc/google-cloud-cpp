// Copyright 2020 Google LLC
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

#include "google/cloud/internal/async_connection_ready.h"
#include "google/cloud/options.h"
#include <chrono>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

// Split up any call into 1s long queries, in case we can back out early.
auto constexpr kDeadlinePerCall = std::chrono::milliseconds(1000);

AsyncConnectionReadyFuture::AsyncConnectionReadyFuture(
    std::shared_ptr<google::cloud::internal::CompletionQueueImpl> cq,
    std::shared_ptr<grpc::Channel> channel,
    std::chrono::system_clock::time_point deadline)
    : cq_(std::move(cq)),
      channel_(std::move(channel)),
      deadline_(deadline),
      promise_([this] { cancelled_ = true; }),
      state_(channel_->GetState(true)) {
}

future<Status> AsyncConnectionReadyFuture::Start() {
  RunIteration();
  return promise_.get_future();
}

void AsyncConnectionReadyFuture::Notify(bool ok) {
  // Weirdly, the `NotifyOnStateChange()` call may return immediately, without
  // the connectivity state having changed. From what I can tell this happens
  // for "lame channels" that are no longer usable.
  if (!ok && call_deadline_ < std::chrono::system_clock::now()) {
    promise_.set_value(Status(StatusCode::kCancelled, "gRPC is done with us"));
    return;
  }
  if (cancelled_) {
    promise_.set_value(Status(StatusCode::kCancelled,
                              "AsyncWaitConnectionReady cancelled by user"));
    return;
  }
  if (!ok && std::chrono::system_clock::now() > deadline_) {
    promise_.set_value(
        Status(StatusCode::kDeadlineExceeded,
               "Connection couldn't connect before requested deadline"));
    return;
  }
  state_ = channel_->GetState(true);
  if (state_ == GRPC_CHANNEL_READY) {
    promise_.set_value(Status{});
    return;
  }
  if (state_ == GRPC_CHANNEL_SHUTDOWN) {
    promise_.set_value(
        Status(StatusCode::kCancelled,
               "Connection will never succeed because it's shut down."));
    return;
  }
  // If connection was idle, GetState(true) triggered an attempt to connect.
  // Otherwise it is either in state CONNECTING or TRANSIENT_FAILURE, so let's
  // register for a state change.
  RunIteration();
}

void AsyncConnectionReadyFuture::RunIteration() {
  class OnStateChange : public AsyncGrpcOperation {
   public:
    explicit OnStateChange(std::shared_ptr<AsyncConnectionReadyFuture> s)
        : self_(std::move(s)) {}
    bool Notify(bool ok) override {
      OptionsSpan span(options_);
      self_->Notify(ok);
      return true;
    }
    void Cancel() override {}

   private:
    std::shared_ptr<AsyncConnectionReadyFuture> const self_;
    Options options_ = CurrentOptions();
  };

  call_deadline_ = (std::min)(
      deadline_, std::chrono::system_clock::now() + kDeadlinePerCall);
  auto op = std::make_shared<OnStateChange>(shared_from_this());
  cq_->StartOperation(op, [this](void* tag) {
    channel_->NotifyOnStateChange(state_, call_deadline_, &cq_->cq(), tag);
  });
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
