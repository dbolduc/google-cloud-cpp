// Copyright 2021 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_AUTH_DATA_CLIENT_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_AUTH_DATA_CLIENT_H

#include "google/cloud/bigtable/data_client.h"
#include "google/cloud/internal/unified_grpc_credentials.h"
#include "google/cloud/options.h"
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace bigtable {
inline namespace BIGTABLE_CLIENT_NS {
namespace internal {

namespace btproto = ::google::bigtable::v2;

/**
 * Implement a logging DataClient.
 *
 * This implementation does not support multiple threads, or refresh
 * authorization tokens.  In other words, it is extremely bare bones.
 */
class AuthDataClient : public DataClient {
 public:
  AuthDataClient(std::shared_ptr<google::cloud::bigtable::DataClient> child,
                 google::cloud::Options const& options)
      : child_(std::move(child)),
        background_(
            google::cloud::internal::MakeBackgroundThreadsFactory(options)()),
        auth_(google::cloud::internal::CreateAuthenticationStrategy(
            background_->cq(), options)) {}

  std::string const& project_id() const override {
    return child_->project_id();
  }

  std::string const& instance_id() const override {
    return child_->instance_id();
  }

  std::shared_ptr<grpc::Channel> Channel() override {
    return child_->Channel();
  }

  void reset() override { child_->reset(); }

  grpc::Status MutateRow(grpc::ClientContext* context,
                         btproto::MutateRowRequest const& request,
                         btproto::MutateRowResponse* response) override;

  std::unique_ptr<
      grpc::ClientAsyncResponseReaderInterface<btproto::MutateRowResponse>>
  AsyncMutateRow(grpc::ClientContext* context,
                 btproto::MutateRowRequest const& request,
                 grpc::CompletionQueue* cq) override;

  grpc::Status CheckAndMutateRow(
      grpc::ClientContext* context,
      btproto::CheckAndMutateRowRequest const& request,
      btproto::CheckAndMutateRowResponse* response) override;

  std::unique_ptr<grpc::ClientAsyncResponseReaderInterface<
      btproto::CheckAndMutateRowResponse>>
  AsyncCheckAndMutateRow(grpc::ClientContext* context,
                         btproto::CheckAndMutateRowRequest const& request,
                         grpc::CompletionQueue* cq) override;

  grpc::Status ReadModifyWriteRow(
      grpc::ClientContext* context,
      btproto::ReadModifyWriteRowRequest const& request,
      btproto::ReadModifyWriteRowResponse* response) override;

  std::unique_ptr<grpc::ClientAsyncResponseReaderInterface<
      btproto::ReadModifyWriteRowResponse>>
  AsyncReadModifyWriteRow(grpc::ClientContext* context,
                          btproto::ReadModifyWriteRowRequest const& request,
                          grpc::CompletionQueue* cq) override;

  std::unique_ptr<grpc::ClientReaderInterface<btproto::ReadRowsResponse>>
  ReadRows(grpc::ClientContext* context,
           btproto::ReadRowsRequest const& request) override;

  std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::ReadRowsResponse>>
  AsyncReadRows(grpc::ClientContext* context,
                btproto::ReadRowsRequest const& request,
                grpc::CompletionQueue* cq, void* tag) override;

  std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::ReadRowsResponse>>
  PrepareAsyncReadRows(grpc::ClientContext* context,
                       btproto::ReadRowsRequest const& request,
                       grpc::CompletionQueue* cq) override;

  std::unique_ptr<grpc::ClientReaderInterface<btproto::SampleRowKeysResponse>>
  SampleRowKeys(grpc::ClientContext* context,
                btproto::SampleRowKeysRequest const& request) override;

  std::unique_ptr<
      grpc::ClientAsyncReaderInterface<btproto::SampleRowKeysResponse>>
  AsyncSampleRowKeys(grpc::ClientContext* context,
                     btproto::SampleRowKeysRequest const& request,
                     grpc::CompletionQueue* cq, void* tag) override;
  std::unique_ptr<
      grpc::ClientAsyncReaderInterface<btproto::SampleRowKeysResponse>>
  PrepareAsyncSampleRowKeys(grpc::ClientContext* context,
                            btproto::SampleRowKeysRequest const& request,
                            grpc::CompletionQueue* cq) override;

  std::unique_ptr<grpc::ClientReaderInterface<btproto::MutateRowsResponse>>
  MutateRows(grpc::ClientContext* context,
             btproto::MutateRowsRequest const& request) override;

  std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::MutateRowsResponse>>
  AsyncMutateRows(grpc::ClientContext* context,
                  btproto::MutateRowsRequest const& request,
                  grpc::CompletionQueue* cq, void* tag) override;

  std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::MutateRowsResponse>>
  PrepareAsyncMutateRows(grpc::ClientContext* context,
                         btproto::MutateRowsRequest const& request,
                         grpc::CompletionQueue* cq) override;

 private:
  google::cloud::BackgroundThreadsFactory BackgroundThreadsFactory() override {
    return child_->BackgroundThreadsFactory();
  }

  std::shared_ptr<DataClient> child_;
  std::shared_ptr<google::cloud::BackgroundThreads> background_;
  std::shared_ptr<google::cloud::internal::GrpcAuthenticationStrategy> auth_;
};

}  // namespace internal
}  // namespace BIGTABLE_CLIENT_NS
}  // namespace bigtable
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_AUTH_DATA_CLIENT_H
