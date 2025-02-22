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

// Generated by the Codegen C++ plugin.
// If you make any local changes, they will be lost.
// source: google/cloud/aiplatform/v1/notebook_service.proto

#include "google/cloud/aiplatform/v1/internal/notebook_metadata_decorator.h"
#include "google/cloud/grpc_options.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#include "google/cloud/internal/api_client_header.h"
#include "google/cloud/internal/url_encode.h"
#include "google/cloud/status_or.h"
#include <google/cloud/aiplatform/v1/notebook_service.grpc.pb.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace google {
namespace cloud {
namespace aiplatform_v1_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

NotebookServiceMetadata::NotebookServiceMetadata(
    std::shared_ptr<NotebookServiceStub> child,
    std::multimap<std::string, std::string> fixed_metadata,
    std::string api_client_header)
    : child_(std::move(child)),
      fixed_metadata_(std::move(fixed_metadata)),
      api_client_header_(
          api_client_header.empty()
              ? google::cloud::internal::GeneratedLibClientHeader()
              : std::move(api_client_header)) {}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncCreateNotebookRuntimeTemplate(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::CreateNotebookRuntimeTemplateRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->AsyncCreateNotebookRuntimeTemplate(
      cq, std::move(context), std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::CreateNotebookRuntimeTemplate(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::CreateNotebookRuntimeTemplateRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->CreateNotebookRuntimeTemplate(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::NotebookRuntimeTemplate>
NotebookServiceMetadata::GetNotebookRuntimeTemplate(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::GetNotebookRuntimeTemplateRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->GetNotebookRuntimeTemplate(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::ListNotebookRuntimeTemplatesResponse>
NotebookServiceMetadata::ListNotebookRuntimeTemplates(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::ListNotebookRuntimeTemplatesRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->ListNotebookRuntimeTemplates(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncDeleteNotebookRuntimeTemplate(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::DeleteNotebookRuntimeTemplateRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncDeleteNotebookRuntimeTemplate(
      cq, std::move(context), std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::DeleteNotebookRuntimeTemplate(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::DeleteNotebookRuntimeTemplateRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->DeleteNotebookRuntimeTemplate(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::NotebookRuntimeTemplate>
NotebookServiceMetadata::UpdateNotebookRuntimeTemplate(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::UpdateNotebookRuntimeTemplateRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("notebook_runtime_template.name=",
                           internal::UrlEncode(
                               request.notebook_runtime_template().name())));
  return child_->UpdateNotebookRuntimeTemplate(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncAssignNotebookRuntime(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::AssignNotebookRuntimeRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->AsyncAssignNotebookRuntime(cq, std::move(context),
                                            std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::AssignNotebookRuntime(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::AssignNotebookRuntimeRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->AssignNotebookRuntime(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::NotebookRuntime>
NotebookServiceMetadata::GetNotebookRuntime(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::GetNotebookRuntimeRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->GetNotebookRuntime(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::ListNotebookRuntimesResponse>
NotebookServiceMetadata::ListNotebookRuntimes(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::ListNotebookRuntimesRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->ListNotebookRuntimes(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncDeleteNotebookRuntime(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::DeleteNotebookRuntimeRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncDeleteNotebookRuntime(cq, std::move(context),
                                            std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::DeleteNotebookRuntime(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::DeleteNotebookRuntimeRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->DeleteNotebookRuntime(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncUpgradeNotebookRuntime(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::UpgradeNotebookRuntimeRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncUpgradeNotebookRuntime(cq, std::move(context),
                                             std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::UpgradeNotebookRuntime(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::UpgradeNotebookRuntimeRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->UpgradeNotebookRuntime(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncStartNotebookRuntime(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::StartNotebookRuntimeRequest const& request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncStartNotebookRuntime(cq, std::move(context),
                                           std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::StartNotebookRuntime(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::StartNotebookRuntimeRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->StartNotebookRuntime(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncStopNotebookRuntime(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::StopNotebookRuntimeRequest const& request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncStopNotebookRuntime(cq, std::move(context),
                                          std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::StopNotebookRuntime(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::StopNotebookRuntimeRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->StopNotebookRuntime(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncCreateNotebookExecutionJob(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::CreateNotebookExecutionJobRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->AsyncCreateNotebookExecutionJob(cq, std::move(context),
                                                 std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::CreateNotebookExecutionJob(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::CreateNotebookExecutionJobRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->CreateNotebookExecutionJob(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::NotebookExecutionJob>
NotebookServiceMetadata::GetNotebookExecutionJob(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::GetNotebookExecutionJobRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->GetNotebookExecutionJob(context, options, request);
}

StatusOr<google::cloud::aiplatform::v1::ListNotebookExecutionJobsResponse>
NotebookServiceMetadata::ListNotebookExecutionJobs(
    grpc::ClientContext& context, Options const& options,
    google::cloud::aiplatform::v1::ListNotebookExecutionJobsRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("parent=", internal::UrlEncode(request.parent())));
  return child_->ListNotebookExecutionJobs(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncDeleteNotebookExecutionJob(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::aiplatform::v1::DeleteNotebookExecutionJobRequest const&
        request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncDeleteNotebookExecutionJob(cq, std::move(context),
                                                 std::move(options), request);
}

StatusOr<google::longrunning::Operation>
NotebookServiceMetadata::DeleteNotebookExecutionJob(
    grpc::ClientContext& context, Options options,
    google::cloud::aiplatform::v1::DeleteNotebookExecutionJobRequest const&
        request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->DeleteNotebookExecutionJob(context, options, request);
}

StatusOr<google::cloud::location::ListLocationsResponse>
NotebookServiceMetadata::ListLocations(
    grpc::ClientContext& context, Options const& options,
    google::cloud::location::ListLocationsRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->ListLocations(context, options, request);
}

StatusOr<google::cloud::location::Location>
NotebookServiceMetadata::GetLocation(
    grpc::ClientContext& context, Options const& options,
    google::cloud::location::GetLocationRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->GetLocation(context, options, request);
}

StatusOr<google::iam::v1::Policy> NotebookServiceMetadata::SetIamPolicy(
    grpc::ClientContext& context, Options const& options,
    google::iam::v1::SetIamPolicyRequest const& request) {
  SetMetadata(
      context, options,
      absl::StrCat("resource=", internal::UrlEncode(request.resource())));
  return child_->SetIamPolicy(context, options, request);
}

StatusOr<google::iam::v1::Policy> NotebookServiceMetadata::GetIamPolicy(
    grpc::ClientContext& context, Options const& options,
    google::iam::v1::GetIamPolicyRequest const& request) {
  SetMetadata(
      context, options,
      absl::StrCat("resource=", internal::UrlEncode(request.resource())));
  return child_->GetIamPolicy(context, options, request);
}

StatusOr<google::iam::v1::TestIamPermissionsResponse>
NotebookServiceMetadata::TestIamPermissions(
    grpc::ClientContext& context, Options const& options,
    google::iam::v1::TestIamPermissionsRequest const& request) {
  SetMetadata(
      context, options,
      absl::StrCat("resource=", internal::UrlEncode(request.resource())));
  return child_->TestIamPermissions(context, options, request);
}

StatusOr<google::longrunning::ListOperationsResponse>
NotebookServiceMetadata::ListOperations(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::ListOperationsRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->ListOperations(context, options, request);
}

StatusOr<google::longrunning::Operation> NotebookServiceMetadata::GetOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::GetOperationRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->GetOperation(context, options, request);
}

Status NotebookServiceMetadata::DeleteOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::DeleteOperationRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->DeleteOperation(context, options, request);
}

Status NotebookServiceMetadata::CancelOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::CancelOperationRequest const& request) {
  SetMetadata(context, options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->CancelOperation(context, options, request);
}

StatusOr<google::longrunning::Operation> NotebookServiceMetadata::WaitOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::WaitOperationRequest const& request) {
  SetMetadata(context, options);
  return child_->WaitOperation(context, options, request);
}

future<StatusOr<google::longrunning::Operation>>
NotebookServiceMetadata::AsyncGetOperation(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::longrunning::GetOperationRequest const& request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncGetOperation(cq, std::move(context), std::move(options),
                                   request);
}

future<Status> NotebookServiceMetadata::AsyncCancelOperation(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::longrunning::CancelOperationRequest const& request) {
  SetMetadata(*context, *options,
              absl::StrCat("name=", internal::UrlEncode(request.name())));
  return child_->AsyncCancelOperation(cq, std::move(context),
                                      std::move(options), request);
}

void NotebookServiceMetadata::SetMetadata(grpc::ClientContext& context,
                                          Options const& options,
                                          std::string const& request_params) {
  context.AddMetadata("x-goog-request-params", request_params);
  SetMetadata(context, options);
}

void NotebookServiceMetadata::SetMetadata(grpc::ClientContext& context,
                                          Options const& options) {
  google::cloud::internal::SetMetadata(context, options, fixed_metadata_,
                                       api_client_header_);
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace aiplatform_v1_internal
}  // namespace cloud
}  // namespace google
