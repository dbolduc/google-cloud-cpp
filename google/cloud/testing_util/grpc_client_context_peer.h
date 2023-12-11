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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TESTING_UTIL_GRPC_CLIENT_CONTEXT_PEER_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TESTING_UTIL_GRPC_CLIENT_CONTEXT_PEER_H

#include "google/cloud/version.h"
#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <utility>

// Desperate times call for desperate measures.
namespace grpc {
namespace testing {

/*
 * `grpc::ClientContext` is notoriously opaque. The following `friend` class can
 * help us simulate a server returning metadata to the client.
 */
class ClientContextTestPeer {
 public:
  explicit ClientContextTestPeer(ClientContext* context);

  /// Inject metadata to the ClientContext for the test. The test peer
  /// must be alive when a ClientContext::GetServerInitialMetadata is called.
  void AddServerInitialMetadata(std::pair<std::string, std::string> kv);

  /// Inject metadata to the ClientContext for the test. The test peer
  /// must be alive when a ClientContext::GetServerTrailingMetadata is called.
  void AddServerTrailingMetadata(std::pair<std::string, std::string> kv);

 private:
  ClientContext* context_;  // not owned
  std::multimap<std::string, std::string> headers_;
  std::multimap<std::string, std::string> trailers_;
};

}  // namespace testing
}  // namespace grpc

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace testing_util {

using ::grpc::testing::ClientContextTestPeer;

}  // namespace testing_util
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TESTING_UTIL_GRPC_CLIENT_CONTEXT_PEER_H
