// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/bigtable/internal/retry_context.h"
#include "absl/strings/match.h"

namespace google {
namespace cloud {
namespace bigtable_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

RetryContext::RetryContext(std::shared_ptr<Metrics> metrics,
                           std::string const& client_uid,
                           std::string const& method,
                           std::string const& streaming,
                           std::string const& table_name,
                           std::string const& app_profile)
    : metrics_(std::move(metrics)) {
  labels_.client_name = "Bigtable-C++";
  labels_.client_uid = client_uid;
  labels_.method = method;
  labels_.streaming = streaming;
  labels_.table_name = table_name;
  labels_.app_profile = app_profile;
}

void RetryContext::PreCall(grpc::ClientContext& context) {
  attempt_start_ = std::chrono::system_clock::now();
  for (auto const& h : cookies_) {
    context.AddMetadata(h.first, h.second);
  }
  context.AddMetadata("bigtable-attempt", std::to_string(attempt_number_++));
}

void RetryContext::PostCall(grpc::ClientContext const& context) {
  ProcessMetadata(context.GetServerInitialMetadata());
  ProcessMetadata(context.GetServerTrailingMetadata());

  // NOTE : metrics_ should always exist. But I have only instrumented
  // `Apply()`.
  if (!metrics_) return;
  auto attempt_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - attempt_start_);
  metrics_->RecordAttemptLatency(attempt_elapsed.count(), labels_);
}

void RetryContext::OnDone() {
  // NOTE : metrics_ should always exist. But I have only instrumented
  // `Apply()`.
  if (!metrics_) return;
  auto operation_elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - operation_start_);
  metrics_->RecordOperationLatency(operation_elapsed.count(), labels_);
}

void RetryContext::ProcessMetadata(
    std::multimap<grpc::string_ref, grpc::string_ref> const& metadata) {
  for (auto const& kv : metadata) {
    auto key = std::string{kv.first.data(), kv.first.size()};
    if (absl::StartsWith(key, "x-goog-cbt-cookie")) {
      cookies_[std::move(key)] =
          std::string{kv.second.data(), kv.second.size()};
    }
  }

  // TODO : Capture cluster, zone, server_latencies in here.
  // For now we hardcode them.
  labels_.cluster = "test-cluster";
  labels_.zone = "us-east4-1";
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google
