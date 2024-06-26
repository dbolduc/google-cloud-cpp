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
// source: google/cloud/datafusion/v1/datafusion.proto

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_DATAFUSION_V1_MOCKS_MOCK_DATA_FUSION_CONNECTION_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_DATAFUSION_V1_MOCKS_MOCK_DATA_FUSION_CONNECTION_H

#include "google/cloud/datafusion/v1/data_fusion_connection.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace datafusion_v1_mocks {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

/**
 * A class to mock `DataFusionConnection`.
 *
 * Application developers may want to test their code with simulated responses,
 * including errors, from an object of type `DataFusionClient`. To do so,
 * construct an object of type `DataFusionClient` with an instance of this
 * class. Then use the Google Test framework functions to program the behavior
 * of this mock.
 *
 * @see [This example][bq-mock] for how to test your application with GoogleTest.
 * While the example showcases types from the BigQuery library, the underlying
 * principles apply for any pair of `*Client` and `*Connection`.
 *
 * [bq-mock]: @cloud_cpp_docs_link{bigquery,bigquery-read-mock}
 */
class MockDataFusionConnection : public datafusion_v1::DataFusionConnection {
 public:
  MOCK_METHOD(Options, options, (), (override));

  MOCK_METHOD(
      (StreamRange<google::cloud::datafusion::v1::Version>),
      ListAvailableVersions,
      (google::cloud::datafusion::v1::ListAvailableVersionsRequest request),
      (override));

  MOCK_METHOD((StreamRange<google::cloud::datafusion::v1::Instance>),
              ListInstances,
              (google::cloud::datafusion::v1::ListInstancesRequest request),
              (override));

  MOCK_METHOD(
      StatusOr<google::cloud::datafusion::v1::Instance>, GetInstance,
      (google::cloud::datafusion::v1::GetInstanceRequest const& request),
      (override));

  /// Due to additional overloads for this method
  /// `EXPECT_CALL(*mock, CreateInstance)` is now ambiguous. Use
  /// `EXPECT_CALL(*mock, CreateInstance(::testing::_))` instead.
  MOCK_METHOD(
      future<StatusOr<google::cloud::datafusion::v1::Instance>>, CreateInstance,
      (google::cloud::datafusion::v1::CreateInstanceRequest const& request),
      (override));

  MOCK_METHOD(
      StatusOr<google::longrunning::Operation>, CreateInstance,
      (ExperimentalTag, NoAwaitTag,
       google::cloud::datafusion::v1::CreateInstanceRequest const& request),
      (override));

  MOCK_METHOD(future<StatusOr<google::cloud::datafusion::v1::Instance>>,
              CreateInstance,
              (ExperimentalTag,
               google::longrunning::Operation const& operation),
              (override));

  /// Due to additional overloads for this method
  /// `EXPECT_CALL(*mock, DeleteInstance)` is now ambiguous. Use
  /// `EXPECT_CALL(*mock, DeleteInstance(::testing::_))` instead.
  MOCK_METHOD(
      future<StatusOr<google::cloud::datafusion::v1::OperationMetadata>>,
      DeleteInstance,
      (google::cloud::datafusion::v1::DeleteInstanceRequest const& request),
      (override));

  MOCK_METHOD(
      StatusOr<google::longrunning::Operation>, DeleteInstance,
      (ExperimentalTag, NoAwaitTag,
       google::cloud::datafusion::v1::DeleteInstanceRequest const& request),
      (override));

  MOCK_METHOD(
      future<StatusOr<google::cloud::datafusion::v1::OperationMetadata>>,
      DeleteInstance,
      (ExperimentalTag, google::longrunning::Operation const& operation),
      (override));

  /// Due to additional overloads for this method
  /// `EXPECT_CALL(*mock, UpdateInstance)` is now ambiguous. Use
  /// `EXPECT_CALL(*mock, UpdateInstance(::testing::_))` instead.
  MOCK_METHOD(
      future<StatusOr<google::cloud::datafusion::v1::Instance>>, UpdateInstance,
      (google::cloud::datafusion::v1::UpdateInstanceRequest const& request),
      (override));

  MOCK_METHOD(
      StatusOr<google::longrunning::Operation>, UpdateInstance,
      (ExperimentalTag, NoAwaitTag,
       google::cloud::datafusion::v1::UpdateInstanceRequest const& request),
      (override));

  MOCK_METHOD(future<StatusOr<google::cloud::datafusion::v1::Instance>>,
              UpdateInstance,
              (ExperimentalTag,
               google::longrunning::Operation const& operation),
              (override));

  /// Due to additional overloads for this method
  /// `EXPECT_CALL(*mock, RestartInstance)` is now ambiguous. Use
  /// `EXPECT_CALL(*mock, RestartInstance(::testing::_))` instead.
  MOCK_METHOD(
      future<StatusOr<google::cloud::datafusion::v1::Instance>>,
      RestartInstance,
      (google::cloud::datafusion::v1::RestartInstanceRequest const& request),
      (override));

  MOCK_METHOD(
      StatusOr<google::longrunning::Operation>, RestartInstance,
      (ExperimentalTag, NoAwaitTag,
       google::cloud::datafusion::v1::RestartInstanceRequest const& request),
      (override));

  MOCK_METHOD(future<StatusOr<google::cloud::datafusion::v1::Instance>>,
              RestartInstance,
              (ExperimentalTag,
               google::longrunning::Operation const& operation),
              (override));
};

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace datafusion_v1_mocks
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_DATAFUSION_V1_MOCKS_MOCK_DATA_FUSION_CONNECTION_H
