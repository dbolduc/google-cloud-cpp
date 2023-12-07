// Copyright 2023 Google LLC
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

#include "google/cloud/testing_util/grpc_client_context_peer.h"

namespace grpc {
namespace testing {

ClientContextTestPeer::ClientContextTestPeer(ClientContext* context)
    : context_(context) {
  // Avoid asserts when we try to grab the metadata.
  context_->initial_metadata_received_ = true;
}

/// Inject metadata to the ClientContext for the test. The test peer
/// must be alive when a ClientContext::GetServerInitialMetadata is called.
void ClientContextTestPeer::AddServerInitialMetadata(
    std::pair<std::string, std::string> kv) {
  headers_.insert(std::move(kv));
  context_->recv_initial_metadata_.map()->clear();
  for (auto const& kv : headers_) {
    context_->recv_initial_metadata_.map()->insert(
        std::pair<grpc::string_ref, grpc::string_ref>(kv.first.c_str(),
                                                      kv.second.c_str()));
  }
}

/// Inject metadata to the ClientContext for the test. The test peer
/// must be alive when a ClientContext::GetServerTrailingMetadata is called.
void ClientContextTestPeer::AddServerTrailingMetadata(
    std::pair<std::string, std::string> kv) {
  trailers_.insert(std::move(kv));
  context_->trailing_metadata_.map()->clear();
  for (auto const& kv : trailers_) {
    context_->trailing_metadata_.map()->insert(
        std::pair<grpc::string_ref, grpc::string_ref>(kv.first.c_str(),
                                                      kv.second.c_str()));
  }
}

}  // namespace testing
}  // namespace grpc
