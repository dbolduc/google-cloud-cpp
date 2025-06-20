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
// source: google/cloud/compute/disks/v1/disks.proto

#include "google/cloud/compute/disks/v1/disks_client.h"
#include <memory>
#include <utility>

namespace google {
namespace cloud {
namespace compute_disks_v1 {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

DisksClient::DisksClient(std::shared_ptr<DisksConnection> connection,
                         Options opts)
    : connection_(std::move(connection)),
      options_(
          internal::MergeOptions(std::move(opts), connection_->options())) {}
DisksClient::~DisksClient() = default;

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::AddResourcePolicies(
    std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksAddResourcePoliciesRequest const&
        disks_add_resource_policies_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::AddResourcePoliciesRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_add_resource_policies_request_resource() =
      disks_add_resource_policies_request_resource;
  return connection_->AddResourcePolicies(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::AddResourcePolicies(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksAddResourcePoliciesRequest const&
        disks_add_resource_policies_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::AddResourcePoliciesRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_add_resource_policies_request_resource() =
      disks_add_resource_policies_request_resource;
  return connection_->AddResourcePolicies(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::AddResourcePolicies(
    google::cloud::cpp::compute::disks::v1::AddResourcePoliciesRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->AddResourcePolicies(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::AddResourcePolicies(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::AddResourcePoliciesRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->AddResourcePolicies(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::AddResourcePolicies(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->AddResourcePolicies(operation);
}

StreamRange<
    std::pair<std::string, google::cloud::cpp::compute::v1::DisksScopedList>>
DisksClient::AggregatedListDisks(std::string const& project, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::AggregatedListDisksRequest request;
  request.set_project(project);
  return connection_->AggregatedListDisks(request);
}

StreamRange<
    std::pair<std::string, google::cloud::cpp::compute::v1::DisksScopedList>>
DisksClient::AggregatedListDisks(
    google::cloud::cpp::compute::disks::v1::AggregatedListDisksRequest request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->AggregatedListDisks(std::move(request));
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::BulkInsert(
    std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::BulkInsertDiskResource const&
        bulk_insert_disk_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::BulkInsertRequest request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_bulk_insert_disk_resource() = bulk_insert_disk_resource;
  return connection_->BulkInsert(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::BulkInsert(
    NoAwaitTag, std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::BulkInsertDiskResource const&
        bulk_insert_disk_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::BulkInsertRequest request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_bulk_insert_disk_resource() = bulk_insert_disk_resource;
  return connection_->BulkInsert(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::BulkInsert(
    google::cloud::cpp::compute::disks::v1::BulkInsertRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->BulkInsert(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::BulkInsert(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::BulkInsertRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->BulkInsert(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::BulkInsert(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->BulkInsert(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::BulkSetLabels(
    std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::BulkZoneSetLabelsRequest const&
        bulk_zone_set_labels_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::BulkSetLabelsRequest request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_bulk_zone_set_labels_request_resource() =
      bulk_zone_set_labels_request_resource;
  return connection_->BulkSetLabels(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::BulkSetLabels(
    NoAwaitTag, std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::BulkZoneSetLabelsRequest const&
        bulk_zone_set_labels_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::BulkSetLabelsRequest request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_bulk_zone_set_labels_request_resource() =
      bulk_zone_set_labels_request_resource;
  return connection_->BulkSetLabels(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::BulkSetLabels(
    google::cloud::cpp::compute::disks::v1::BulkSetLabelsRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->BulkSetLabels(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::BulkSetLabels(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::BulkSetLabelsRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->BulkSetLabels(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::BulkSetLabels(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->BulkSetLabels(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::CreateSnapshot(
    std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::Snapshot const& snapshot_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::CreateSnapshotRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_snapshot_resource() = snapshot_resource;
  return connection_->CreateSnapshot(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::CreateSnapshot(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::Snapshot const& snapshot_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::CreateSnapshotRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_snapshot_resource() = snapshot_resource;
  return connection_->CreateSnapshot(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::CreateSnapshot(
    google::cloud::cpp::compute::disks::v1::CreateSnapshotRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->CreateSnapshot(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::CreateSnapshot(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::CreateSnapshotRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->CreateSnapshot(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::CreateSnapshot(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->CreateSnapshot(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::DeleteDisk(std::string const& project, std::string const& zone,
                        std::string const& disk, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::DeleteDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  return connection_->DeleteDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::DeleteDisk(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::DeleteDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  return connection_->DeleteDisk(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::DeleteDisk(
    google::cloud::cpp::compute::disks::v1::DeleteDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->DeleteDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::DeleteDisk(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::DeleteDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->DeleteDisk(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::DeleteDisk(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->DeleteDisk(operation);
}

StatusOr<google::cloud::cpp::compute::v1::Disk> DisksClient::GetDisk(
    std::string const& project, std::string const& zone,
    std::string const& disk, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::GetDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  return connection_->GetDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Disk> DisksClient::GetDisk(
    google::cloud::cpp::compute::disks::v1::GetDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->GetDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Policy> DisksClient::GetIamPolicy(
    std::string const& project, std::string const& zone,
    std::string const& resource, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::GetIamPolicyRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_resource(resource);
  return connection_->GetIamPolicy(request);
}

StatusOr<google::cloud::cpp::compute::v1::Policy> DisksClient::GetIamPolicy(
    google::cloud::cpp::compute::disks::v1::GetIamPolicyRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->GetIamPolicy(request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::InsertDisk(
    std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::Disk const& disk_resource, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::InsertDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_disk_resource() = disk_resource;
  return connection_->InsertDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::InsertDisk(
    NoAwaitTag, std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::Disk const& disk_resource, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::InsertDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_disk_resource() = disk_resource;
  return connection_->InsertDisk(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::InsertDisk(
    google::cloud::cpp::compute::disks::v1::InsertDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->InsertDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::InsertDisk(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::InsertDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->InsertDisk(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::InsertDisk(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->InsertDisk(operation);
}

StreamRange<google::cloud::cpp::compute::v1::Disk> DisksClient::ListDisks(
    std::string const& project, std::string const& zone, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::ListDisksRequest request;
  request.set_project(project);
  request.set_zone(zone);
  return connection_->ListDisks(request);
}

StreamRange<google::cloud::cpp::compute::v1::Disk> DisksClient::ListDisks(
    google::cloud::cpp::compute::disks::v1::ListDisksRequest request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->ListDisks(std::move(request));
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::RemoveResourcePolicies(
    std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksRemoveResourcePoliciesRequest const&
        disks_remove_resource_policies_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::RemoveResourcePoliciesRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_remove_resource_policies_request_resource() =
      disks_remove_resource_policies_request_resource;
  return connection_->RemoveResourcePolicies(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::RemoveResourcePolicies(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksRemoveResourcePoliciesRequest const&
        disks_remove_resource_policies_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::RemoveResourcePoliciesRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_remove_resource_policies_request_resource() =
      disks_remove_resource_policies_request_resource;
  return connection_->RemoveResourcePolicies(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::RemoveResourcePolicies(
    google::cloud::cpp::compute::disks::v1::RemoveResourcePoliciesRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->RemoveResourcePolicies(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::RemoveResourcePolicies(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::RemoveResourcePoliciesRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->RemoveResourcePolicies(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::RemoveResourcePolicies(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->RemoveResourcePolicies(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::Resize(std::string const& project, std::string const& zone,
                    std::string const& disk,
                    google::cloud::cpp::compute::v1::DisksResizeRequest const&
                        disks_resize_request_resource,
                    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::ResizeRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_resize_request_resource() =
      disks_resize_request_resource;
  return connection_->Resize(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::Resize(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksResizeRequest const&
        disks_resize_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::ResizeRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_resize_request_resource() =
      disks_resize_request_resource;
  return connection_->Resize(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::Resize(
    google::cloud::cpp::compute::disks::v1::ResizeRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->Resize(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::Resize(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::ResizeRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->Resize(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::Resize(google::cloud::cpp::compute::v1::Operation const& operation,
                    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->Resize(operation);
}

StatusOr<google::cloud::cpp::compute::v1::Policy> DisksClient::SetIamPolicy(
    std::string const& project, std::string const& zone,
    std::string const& resource,
    google::cloud::cpp::compute::v1::ZoneSetPolicyRequest const&
        zone_set_policy_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::SetIamPolicyRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_resource(resource);
  *request.mutable_zone_set_policy_request_resource() =
      zone_set_policy_request_resource;
  return connection_->SetIamPolicy(request);
}

StatusOr<google::cloud::cpp::compute::v1::Policy> DisksClient::SetIamPolicy(
    google::cloud::cpp::compute::disks::v1::SetIamPolicyRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->SetIamPolicy(request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::SetLabels(
    std::string const& project, std::string const& zone,
    std::string const& resource,
    google::cloud::cpp::compute::v1::ZoneSetLabelsRequest const&
        zone_set_labels_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::SetLabelsRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_resource(resource);
  *request.mutable_zone_set_labels_request_resource() =
      zone_set_labels_request_resource;
  return connection_->SetLabels(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::SetLabels(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& resource,
    google::cloud::cpp::compute::v1::ZoneSetLabelsRequest const&
        zone_set_labels_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::SetLabelsRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_resource(resource);
  *request.mutable_zone_set_labels_request_resource() =
      zone_set_labels_request_resource;
  return connection_->SetLabels(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::SetLabels(
    google::cloud::cpp::compute::disks::v1::SetLabelsRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->SetLabels(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::SetLabels(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::SetLabelsRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->SetLabels(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::SetLabels(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->SetLabels(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StartAsyncReplication(
    std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksStartAsyncReplicationRequest const&
        disks_start_async_replication_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::StartAsyncReplicationRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_start_async_replication_request_resource() =
      disks_start_async_replication_request_resource;
  return connection_->StartAsyncReplication(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::StartAsyncReplication(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk,
    google::cloud::cpp::compute::v1::DisksStartAsyncReplicationRequest const&
        disks_start_async_replication_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::StartAsyncReplicationRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  *request.mutable_disks_start_async_replication_request_resource() =
      disks_start_async_replication_request_resource;
  return connection_->StartAsyncReplication(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StartAsyncReplication(
    google::cloud::cpp::compute::disks::v1::StartAsyncReplicationRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StartAsyncReplication(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::StartAsyncReplication(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::StartAsyncReplicationRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StartAsyncReplication(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StartAsyncReplication(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StartAsyncReplication(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StopAsyncReplication(std::string const& project,
                                  std::string const& zone,
                                  std::string const& disk, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::StopAsyncReplicationRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  return connection_->StopAsyncReplication(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::StopAsyncReplication(NoAwaitTag, std::string const& project,
                                  std::string const& zone,
                                  std::string const& disk, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::StopAsyncReplicationRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  return connection_->StopAsyncReplication(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StopAsyncReplication(
    google::cloud::cpp::compute::disks::v1::StopAsyncReplicationRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StopAsyncReplication(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::StopAsyncReplication(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::StopAsyncReplicationRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StopAsyncReplication(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StopAsyncReplication(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StopAsyncReplication(operation);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StopGroupAsyncReplication(
    std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::
        DisksStopGroupAsyncReplicationResource const&
            disks_stop_group_async_replication_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::StopGroupAsyncReplicationRequest
      request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_disks_stop_group_async_replication_resource() =
      disks_stop_group_async_replication_resource;
  return connection_->StopGroupAsyncReplication(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::StopGroupAsyncReplication(
    NoAwaitTag, std::string const& project, std::string const& zone,
    google::cloud::cpp::compute::v1::
        DisksStopGroupAsyncReplicationResource const&
            disks_stop_group_async_replication_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::StopGroupAsyncReplicationRequest
      request;
  request.set_project(project);
  request.set_zone(zone);
  *request.mutable_disks_stop_group_async_replication_resource() =
      disks_stop_group_async_replication_resource;
  return connection_->StopGroupAsyncReplication(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StopGroupAsyncReplication(
    google::cloud::cpp::compute::disks::v1::
        StopGroupAsyncReplicationRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StopGroupAsyncReplication(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation>
DisksClient::StopGroupAsyncReplication(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::
        StopGroupAsyncReplicationRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StopGroupAsyncReplication(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::StopGroupAsyncReplication(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->StopGroupAsyncReplication(operation);
}

StatusOr<google::cloud::cpp::compute::v1::TestPermissionsResponse>
DisksClient::TestIamPermissions(
    std::string const& project, std::string const& zone,
    std::string const& resource,
    google::cloud::cpp::compute::v1::TestPermissionsRequest const&
        test_permissions_request_resource,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::TestIamPermissionsRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_resource(resource);
  *request.mutable_test_permissions_request_resource() =
      test_permissions_request_resource;
  return connection_->TestIamPermissions(request);
}

StatusOr<google::cloud::cpp::compute::v1::TestPermissionsResponse>
DisksClient::TestIamPermissions(
    google::cloud::cpp::compute::disks::v1::TestIamPermissionsRequest const&
        request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->TestIamPermissions(request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::UpdateDisk(
    std::string const& project, std::string const& zone,
    std::string const& disk, std::string const& update_mask,
    google::cloud::cpp::compute::v1::Disk const& disk_resource, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::UpdateDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  request.set_update_mask(update_mask);
  *request.mutable_disk_resource() = disk_resource;
  return connection_->UpdateDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::UpdateDisk(
    NoAwaitTag, std::string const& project, std::string const& zone,
    std::string const& disk, std::string const& update_mask,
    google::cloud::cpp::compute::v1::Disk const& disk_resource, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  google::cloud::cpp::compute::disks::v1::UpdateDiskRequest request;
  request.set_project(project);
  request.set_zone(zone);
  request.set_disk(disk);
  request.set_update_mask(update_mask);
  *request.mutable_disk_resource() = disk_resource;
  return connection_->UpdateDisk(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::UpdateDisk(
    google::cloud::cpp::compute::disks::v1::UpdateDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->UpdateDisk(request);
}

StatusOr<google::cloud::cpp::compute::v1::Operation> DisksClient::UpdateDisk(
    NoAwaitTag,
    google::cloud::cpp::compute::disks::v1::UpdateDiskRequest const& request,
    Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->UpdateDisk(NoAwaitTag{}, request);
}

future<StatusOr<google::cloud::cpp::compute::v1::Operation>>
DisksClient::UpdateDisk(
    google::cloud::cpp::compute::v1::Operation const& operation, Options opts) {
  internal::OptionsSpan span(internal::MergeOptions(std::move(opts), options_));
  return connection_->UpdateDisk(operation);
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace compute_disks_v1
}  // namespace cloud
}  // namespace google
