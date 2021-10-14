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

#include "google/cloud/bigtable/internal/auth_data_client.h"
#include "google/cloud/bigtable/internal/common_client.h"
#include "google/cloud/internal/log_wrapper.h"
#include "google/cloud/log.h"
#include "google/cloud/tracing_options.h"
#include <google/longrunning/operations.grpc.pb.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/sync_stream.h>

// TODO : Darren
#include "google/cloud/internal/streaming_read_rpc.h"
#include "google/bigtable/v2/bigtable.pb.h"
namespace {
grpc::Status ToGrpcStatus(google::cloud::Status) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "TODO : Darren");
}
}  // namespace

namespace google {
namespace cloud {
namespace bigtable {
inline namespace BIGTABLE_CLIENT_NS {
namespace internal {

namespace btproto = ::google::bigtable::v2;
using ::google::cloud::internal::LogWrapper;

grpc::Status AuthDataClient::MutateRow(grpc::ClientContext* context,
                                       btproto::MutateRowRequest const& request,
                                       btproto::MutateRowResponse* response) {
  // NOTE :
  // Synchronous case (easy)

  auto auth_status = auth_->ConfigureContext(*context);
  if (!auth_status.ok()) return ToGrpcStatus(auth_status);
  return child_->MutateRow(context, request, response);
}

std::unique_ptr<
    grpc::ClientAsyncResponseReaderInterface<btproto::MutateRowResponse>>
AuthDataClient::AsyncMutateRow(grpc::ClientContext* context,
                               btproto::MutateRowRequest const& request,
                               grpc::CompletionQueue* cq) {
  // NOTE :
  // Asynchronous Case (not quite as easy)

  using ReaderType =
      grpc::ClientAsyncResponseReaderInterface<btproto::MutateRowResponse>;
  using ReturnType = std::unique_ptr<ReaderType>;
  using StreamFactory = std::function<ReturnType(grpc::ClientContext*)>;

  // TODO : template this on the response type
  class AsyncStream
      : grpc::ClientAsyncResponseReaderInterface<btproto::MutateRowResponse> {
   public:
    explicit AsyncStream(
        grpc::ClientContext* context,
        std::shared_ptr<google::cloud::internal::GrpcAuthenticationStrategy>
            auth,
        StreamFactory factory);  // TODO

    void StartCall() override {}
    void ReadInitialMetadata(void* tag) override {}
    void Finish(btproto::MutateRowResponse* msg, grpc::Status* status,
                void* tag) override {
      // Call auth_->AsyncConfigureContext
      // If error, return an error stream (of type: `ReturnType`) that just
      //        fills `status` with an error.
      // Else, call the factory to generate a stream
    }
  };

  auto child = child_;
  auto call = [child, cq, request](grpc::ClientContext* ctx) mutable {
    return child->AsyncMutateRow(ctx, request, cq);
  };

  // NOTE :
  // This almost certainly doesn't work. The whole argument is that it isn't
  // worth taking the time to figure out why this doesn't work when we don't
  // want to end up with an implementation that looks like this anyway.
  auto stream = std::make_shared<AsyncStream>(context, auth_, call);
  return absl::make_unique<ReaderType>(stream);
}

grpc::Status AuthDataClient::CheckAndMutateRow(
    grpc::ClientContext* context,
    btproto::CheckAndMutateRowRequest const& request,
    btproto::CheckAndMutateRowResponse* response) {
  return child_->CheckAndMutateRow(context, request, response);
}

std::unique_ptr<grpc::ClientAsyncResponseReaderInterface<
    btproto::CheckAndMutateRowResponse>>
AuthDataClient::AsyncCheckAndMutateRow(
    grpc::ClientContext* context,
    btproto::CheckAndMutateRowRequest const& request,
    grpc::CompletionQueue* cq) {
  return child_->AsyncCheckAndMutateRow(context, request, cq);
}

grpc::Status AuthDataClient::ReadModifyWriteRow(
    grpc::ClientContext* context,
    btproto::ReadModifyWriteRowRequest const& request,
    btproto::ReadModifyWriteRowResponse* response) {
  return child_->ReadModifyWriteRow(context, request, response);
}

std::unique_ptr<grpc::ClientAsyncResponseReaderInterface<
    btproto::ReadModifyWriteRowResponse>>
AuthDataClient::AsyncReadModifyWriteRow(
    grpc::ClientContext* context,
    btproto::ReadModifyWriteRowRequest const& request,
    grpc::CompletionQueue* cq) {
  return child_->AsyncReadModifyWriteRow(context, request, cq);
}

std::unique_ptr<grpc::ClientReaderInterface<btproto::ReadRowsResponse>>
AuthDataClient::ReadRows(grpc::ClientContext* context,
                         btproto::ReadRowsRequest const& request) {
  // NOTE :
  // Synchronous streaming case

  // TODO : template this on the response type
  class ErrorStream : grpc::ClientReaderInterface<btproto::ReadRowsResponse> {
   public:
    explicit ErrorStream(Status s) : status_(ToGrpcStatus(std::move(s))) {}

    void WaitForInitialMetadata() override {}
    bool NextMessageSize(uint32_t*) override { return false; }
    grpc::Status Finish() override { return status_; }
    bool Read(btproto::ReadRowsResponse*) override { return false; }

    using BaseType = grpc::ClientReaderInterface<btproto::ReadRowsResponse>;
    using ReturnType = std::unique_ptr<BaseType>;
    ReturnType clone() { return absl::make_unique<BaseType>(*this); }

   private:
    grpc::Status status_;
  };

  auto status = auth_->ConfigureContext(*context);
  if (!status.ok()) return ErrorStream(std::move(status)).clone();
  return child_->ReadRows(context, request);
}

std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::ReadRowsResponse>>
AuthDataClient::AsyncReadRows(grpc::ClientContext* context,
                              btproto::ReadRowsRequest const& request,
                              grpc::CompletionQueue* cq, void* tag) {
  return child_->AsyncReadRows(context, request, cq, tag);
}

std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::ReadRowsResponse>>
AuthDataClient::PrepareAsyncReadRows(grpc::ClientContext* context,
                                     btproto::ReadRowsRequest const& request,
                                     grpc::CompletionQueue* cq) {
  return child_->PrepareAsyncReadRows(context, request, cq);
}

std::unique_ptr<grpc::ClientReaderInterface<btproto::SampleRowKeysResponse>>
AuthDataClient::SampleRowKeys(grpc::ClientContext* context,
                              btproto::SampleRowKeysRequest const& request) {
  return child_->SampleRowKeys(context, request);
}

std::unique_ptr<
    grpc::ClientAsyncReaderInterface<btproto::SampleRowKeysResponse>>
AuthDataClient::AsyncSampleRowKeys(grpc::ClientContext* context,
                                   btproto::SampleRowKeysRequest const& request,
                                   grpc::CompletionQueue* cq, void* tag) {
  return child_->AsyncSampleRowKeys(context, request, cq, tag);
}

std::unique_ptr<
    grpc::ClientAsyncReaderInterface<btproto::SampleRowKeysResponse>>
AuthDataClient::PrepareAsyncSampleRowKeys(
    grpc::ClientContext* context, btproto::SampleRowKeysRequest const& request,
    grpc::CompletionQueue* cq) {
  return child_->PrepareAsyncSampleRowKeys(context, request, cq);
}

std::unique_ptr<grpc::ClientReaderInterface<btproto::MutateRowsResponse>>
AuthDataClient::MutateRows(grpc::ClientContext* context,
                           btproto::MutateRowsRequest const& request) {
  return child_->MutateRows(context, request);
}

std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::MutateRowsResponse>>
AuthDataClient::AsyncMutateRows(grpc::ClientContext* context,
                                btproto::MutateRowsRequest const& request,
                                grpc::CompletionQueue* cq, void* tag) {
  return child_->AsyncMutateRows(context, request, cq, tag);
}

std::unique_ptr<grpc::ClientAsyncReaderInterface<btproto::MutateRowsResponse>>
AuthDataClient::PrepareAsyncMutateRows(
    grpc::ClientContext* context, btproto::MutateRowsRequest const& request,
    grpc::CompletionQueue* cq) {
  return child_->PrepareAsyncMutateRows(context, request, cq);
}

}  // namespace internal
}  // namespace BIGTABLE_CLIENT_NS
}  // namespace bigtable
}  // namespace cloud
}  // namespace google
