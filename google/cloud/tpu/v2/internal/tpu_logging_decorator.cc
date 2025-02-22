// Copyright 2023 Google LLC
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
// source: google/cloud/tpu/v2/cloud_tpu.proto

#include "google/cloud/tpu/v2/internal/tpu_logging_decorator.h"
#include "google/cloud/internal/log_wrapper.h"
#include "google/cloud/status_or.h"
#include <google/cloud/tpu/v2/cloud_tpu.grpc.pb.h>
#include <memory>
#include <set>
#include <string>
#include <utility>

namespace google {
namespace cloud {
namespace tpu_v2_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

TpuLogging::TpuLogging(std::shared_ptr<TpuStub> child,
                       TracingOptions tracing_options,
                       std::set<std::string> const&)
    : child_(std::move(child)), tracing_options_(std::move(tracing_options)) {}

StatusOr<google::cloud::tpu::v2::ListNodesResponse> TpuLogging::ListNodes(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::ListNodesRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::ListNodesRequest const& request) {
        return child_->ListNodes(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::Node> TpuLogging::GetNode(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::GetNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::GetNodeRequest const& request) {
        return child_->GetNode(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>> TpuLogging::AsyncCreateNode(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::CreateNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::cloud::tpu::v2::CreateNodeRequest const& request) {
        return child_->AsyncCreateNode(cq, std::move(context),
                                       std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::CreateNode(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::CreateNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::CreateNodeRequest const& request) {
        return child_->CreateNode(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>> TpuLogging::AsyncDeleteNode(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::DeleteNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::cloud::tpu::v2::DeleteNodeRequest const& request) {
        return child_->AsyncDeleteNode(cq, std::move(context),
                                       std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::DeleteNode(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::DeleteNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::DeleteNodeRequest const& request) {
        return child_->DeleteNode(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>> TpuLogging::AsyncStopNode(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::StopNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::cloud::tpu::v2::StopNodeRequest const& request) {
        return child_->AsyncStopNode(cq, std::move(context), std::move(options),
                                     request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::StopNode(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::StopNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::StopNodeRequest const& request) {
        return child_->StopNode(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>> TpuLogging::AsyncStartNode(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::StartNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::cloud::tpu::v2::StartNodeRequest const& request) {
        return child_->AsyncStartNode(cq, std::move(context),
                                      std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::StartNode(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::StartNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::StartNodeRequest const& request) {
        return child_->StartNode(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>> TpuLogging::AsyncUpdateNode(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::UpdateNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::cloud::tpu::v2::UpdateNodeRequest const& request) {
        return child_->AsyncUpdateNode(cq, std::move(context),
                                       std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::UpdateNode(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::UpdateNodeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::UpdateNodeRequest const& request) {
        return child_->UpdateNode(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::ListQueuedResourcesResponse>
TpuLogging::ListQueuedResources(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::ListQueuedResourcesRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          grpc::ClientContext& context, Options const& options,
          google::cloud::tpu::v2::ListQueuedResourcesRequest const& request) {
        return child_->ListQueuedResources(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::QueuedResource> TpuLogging::GetQueuedResource(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::GetQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::GetQueuedResourceRequest const& request) {
        return child_->GetQueuedResource(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>>
TpuLogging::AsyncCreateQueuedResource(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::CreateQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          google::cloud::CompletionQueue& cq,
          std::shared_ptr<grpc::ClientContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::tpu::v2::CreateQueuedResourceRequest const& request) {
        return child_->AsyncCreateQueuedResource(cq, std::move(context),
                                                 std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::CreateQueuedResource(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::CreateQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          grpc::ClientContext& context, Options const& options,
          google::cloud::tpu::v2::CreateQueuedResourceRequest const& request) {
        return child_->CreateQueuedResource(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>>
TpuLogging::AsyncDeleteQueuedResource(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::DeleteQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          google::cloud::CompletionQueue& cq,
          std::shared_ptr<grpc::ClientContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::tpu::v2::DeleteQueuedResourceRequest const& request) {
        return child_->AsyncDeleteQueuedResource(cq, std::move(context),
                                                 std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::DeleteQueuedResource(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::DeleteQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          grpc::ClientContext& context, Options const& options,
          google::cloud::tpu::v2::DeleteQueuedResourceRequest const& request) {
        return child_->DeleteQueuedResource(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>>
TpuLogging::AsyncResetQueuedResource(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::cloud::tpu::v2::ResetQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          google::cloud::CompletionQueue& cq,
          std::shared_ptr<grpc::ClientContext> context,
          google::cloud::internal::ImmutableOptions options,
          google::cloud::tpu::v2::ResetQueuedResourceRequest const& request) {
        return child_->AsyncResetQueuedResource(cq, std::move(context),
                                                std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::ResetQueuedResource(
    grpc::ClientContext& context, Options options,
    google::cloud::tpu::v2::ResetQueuedResourceRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          grpc::ClientContext& context, Options const& options,
          google::cloud::tpu::v2::ResetQueuedResourceRequest const& request) {
        return child_->ResetQueuedResource(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::GenerateServiceIdentityResponse>
TpuLogging::GenerateServiceIdentity(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::GenerateServiceIdentityRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::GenerateServiceIdentityRequest const&
                 request) {
        return child_->GenerateServiceIdentity(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::ListAcceleratorTypesResponse>
TpuLogging::ListAcceleratorTypes(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::ListAcceleratorTypesRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          grpc::ClientContext& context, Options const& options,
          google::cloud::tpu::v2::ListAcceleratorTypesRequest const& request) {
        return child_->ListAcceleratorTypes(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::AcceleratorType>
TpuLogging::GetAcceleratorType(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::GetAcceleratorTypeRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::GetAcceleratorTypeRequest const& request) {
        return child_->GetAcceleratorType(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::ListRuntimeVersionsResponse>
TpuLogging::ListRuntimeVersions(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::ListRuntimeVersionsRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](
          grpc::ClientContext& context, Options const& options,
          google::cloud::tpu::v2::ListRuntimeVersionsRequest const& request) {
        return child_->ListRuntimeVersions(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::RuntimeVersion> TpuLogging::GetRuntimeVersion(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::GetRuntimeVersionRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::GetRuntimeVersionRequest const& request) {
        return child_->GetRuntimeVersion(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::tpu::v2::GetGuestAttributesResponse>
TpuLogging::GetGuestAttributes(
    grpc::ClientContext& context, Options const& options,
    google::cloud::tpu::v2::GetGuestAttributesRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::tpu::v2::GetGuestAttributesRequest const& request) {
        return child_->GetGuestAttributes(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::location::ListLocationsResponse>
TpuLogging::ListLocations(
    grpc::ClientContext& context, Options const& options,
    google::cloud::location::ListLocationsRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::location::ListLocationsRequest const& request) {
        return child_->ListLocations(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::cloud::location::Location> TpuLogging::GetLocation(
    grpc::ClientContext& context, Options const& options,
    google::cloud::location::GetLocationRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::cloud::location::GetLocationRequest const& request) {
        return child_->GetLocation(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::longrunning::ListOperationsResponse>
TpuLogging::ListOperations(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::ListOperationsRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::longrunning::ListOperationsRequest const& request) {
        return child_->ListOperations(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

StatusOr<google::longrunning::Operation> TpuLogging::GetOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::GetOperationRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::longrunning::GetOperationRequest const& request) {
        return child_->GetOperation(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

Status TpuLogging::DeleteOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::DeleteOperationRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::longrunning::DeleteOperationRequest const& request) {
        return child_->DeleteOperation(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

Status TpuLogging::CancelOperation(
    grpc::ClientContext& context, Options const& options,
    google::longrunning::CancelOperationRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](grpc::ClientContext& context, Options const& options,
             google::longrunning::CancelOperationRequest const& request) {
        return child_->CancelOperation(context, options, request);
      },
      context, options, request, __func__, tracing_options_);
}

future<StatusOr<google::longrunning::Operation>> TpuLogging::AsyncGetOperation(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::longrunning::GetOperationRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::longrunning::GetOperationRequest const& request) {
        return child_->AsyncGetOperation(cq, std::move(context),
                                         std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

future<Status> TpuLogging::AsyncCancelOperation(
    google::cloud::CompletionQueue& cq,
    std::shared_ptr<grpc::ClientContext> context,
    google::cloud::internal::ImmutableOptions options,
    google::longrunning::CancelOperationRequest const& request) {
  return google::cloud::internal::LogWrapper(
      [this](google::cloud::CompletionQueue& cq,
             std::shared_ptr<grpc::ClientContext> context,
             google::cloud::internal::ImmutableOptions options,
             google::longrunning::CancelOperationRequest const& request) {
        return child_->AsyncCancelOperation(cq, std::move(context),
                                            std::move(options), request);
      },
      cq, std::move(context), std::move(options), request, __func__,
      tracing_options_);
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace tpu_v2_internal
}  // namespace cloud
}  // namespace google
