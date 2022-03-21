// Copyright 2022 Google LLC
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

#include "google/cloud/bigtable/internal/bigtable_connection_impl.h"
#include "google/cloud/bigtable/testing/mock_bigtable_stub.h"
#include "google/cloud/grpc_options.h"
#include "google/cloud/testing_util/status_matchers.h"
#include <gmock/gmock.h>
#include <grpcpp/client_context.h>
#include <chrono>

namespace google {
namespace cloud {
namespace bigtable_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::bigtable::testing::MockBigtableStub;
using ::google::cloud::testing_util::StatusIs;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::MockFunction;
using ::testing::Return;

namespace btproto = ::google::bigtable::v2;

// TODO : refactor this in common?
class MockBackoffPolicy : public BackoffPolicy {
 public:
  MOCK_METHOD(std::unique_ptr<BackoffPolicy>, clone, (), (const, override));
  MOCK_METHOD(std::chrono::milliseconds, OnCompletion, (), (override));
};

// TODO : Note to self : borrowed this from async_row_sampler_test.cc
struct RowKeySampleVectors {
  explicit RowKeySampleVectors(std::vector<google::bigtable::v2::SampleRowKeysResponse> responses) {
    row_keys.reserve(responses.size());
    offset_bytes.reserve(responses.size());
    for (auto& r : responses) {
      row_keys.emplace_back(std::move(*r.mutable_row_key()));
      offset_bytes.emplace_back(r.offset_bytes());
    }
  }

  std::vector<std::string> row_keys;
  std::vector<std::int64_t> offset_bytes;
};

class BigtableConnectionTest : public ::testing::Test {
 protected:
  // Use mocks to validate the backoff policy, the gRPC Setup() functionality.
  // We pass these into the Connection layer via an OptionsSpan.
  Options TestCallOptions(int expected_retries) {
    EXPECT_CALL(*mock_backoff_, clone).WillOnce([expected_retries] {
      auto clone = absl::make_unique<MockBackoffPolicy>();
      EXPECT_CALL(*clone, OnCompletion)
          .Times(expected_retries)
          .WillRepeatedly(Return(std::chrono::milliseconds(0)));
      return clone;
    });

    // TODO : Previously this was called exactly once per full client operation.
    //        But now we must call it once per attempt....
    //
    // Ensure that the grpc::ClientContext is configured for each call.
    EXPECT_CALL(mock_setup_, Call).Times(expected_retries + 1);

    return Options{}
        .set<internal::GrpcSetupOption>(mock_setup_.AsStdFunction())
        .set<bigtable::BigtableBackoffPolicyOption>(mock_backoff_);
  }

  std::shared_ptr<BigtableConnectionImpl> TestConnection() {
    auto options =
        Options{}.set<GrpcCredentialOption>(grpc::InsecureChannelCredentials());
    auto background = internal::MakeBackgroundThreadsFactory(options)();
    return std::make_shared<BigtableConnectionImpl>(
        std::move(background), mock_stub_, std::move(options));
  }

  std::shared_ptr<MockBigtableStub> mock_stub_ =
      std::make_shared<MockBigtableStub>();
  std::shared_ptr<MockBackoffPolicy> mock_backoff_ =
      std::make_shared<MockBackoffPolicy>();
  MockFunction<void(grpc::ClientContext&)> mock_setup_;
};

TEST_F(BigtableConnectionTest, SampleRowsSuccess) {
  // Read two samples then return OK.
  EXPECT_CALL(*mock_stub_, SampleRowKeys)
      .WillOnce([](std::unique_ptr<grpc::ClientContext>,
                   google::bigtable::v2::SampleRowKeysRequest const&) {
        auto reader = absl::make_unique<
            bigtable::testing::MockSampleRowsStreamingReadRpc>();
        EXPECT_CALL(*reader, Read)
            .WillOnce([] {
              google::bigtable::v2::SampleRowKeysResponse resp;
              resp.set_row_key("r1");
              resp.set_offset_bytes(1);
              return resp;
            })
            .WillOnce([] {
              google::bigtable::v2::SampleRowKeysResponse resp;
              resp.set_row_key("r2");
              resp.set_offset_bytes(2);
              return resp;
            })
            .WillOnce(Return(Status()));
        return reader;
      });

  internal::OptionsSpan span(TestCallOptions(/*expected_retries=*/0));
  auto connection = TestConnection();
  auto resp = connection->SampleRowKeys({});
  ASSERT_STATUS_OK(resp);
  auto samples = RowKeySampleVectors(*resp);
  EXPECT_THAT(samples.row_keys, ElementsAre("r1", "r2"));
  EXPECT_THAT(samples.offset_bytes, ElementsAre(1, 2));
}

TEST_F(BigtableConnectionTest, SampleRowsPermanentFailure) {
  EXPECT_CALL(*mock_stub_, SampleRowKeys)
      .WillOnce([](std::unique_ptr<grpc::ClientContext>,
                   google::bigtable::v2::SampleRowKeysRequest const&) {
        auto reader = absl::make_unique<
            bigtable::testing::MockSampleRowsStreamingReadRpc>();
        EXPECT_CALL(*reader, Read)
            .WillOnce(Return(Status(StatusCode::kPermissionDenied, "uh oh")));
        return reader;
      });

  internal::OptionsSpan span(TestCallOptions(/*expected_retries=*/0));
  auto connection = TestConnection();
  auto resp = connection->SampleRowKeys({});
  EXPECT_THAT(
      resp, StatusIs(StatusCode::kPermissionDenied,
                     AllOf(HasSubstr("Permanent error:"), HasSubstr("uh oh"))));
}

TEST_F(BigtableConnectionTest, SampleRowsRetryThenSuccess) {
  // Return a stream that ends in a transient error, then one that succeeds. We
  // should discard any samples returned by the failing stream.
  EXPECT_CALL(*mock_stub_, SampleRowKeys)
      .WillOnce([](std::unique_ptr<grpc::ClientContext>,
                   google::bigtable::v2::SampleRowKeysRequest const&) {
        auto reader = absl::make_unique<
            bigtable::testing::MockSampleRowsStreamingReadRpc>();
        EXPECT_CALL(*reader, Read)
            .WillOnce([] {
              google::bigtable::v2::SampleRowKeysResponse resp;
              resp.set_row_key("discarded-row-key");
              resp.set_offset_bytes(1);
              return resp;
            })
            .WillOnce(Return(Status(StatusCode::kUnavailable, "try again")));
        return reader;
      })
      .WillOnce([](std::unique_ptr<grpc::ClientContext>,
                   google::bigtable::v2::SampleRowKeysRequest const&) {
        auto reader = absl::make_unique<
            bigtable::testing::MockSampleRowsStreamingReadRpc>();
        EXPECT_CALL(*reader, Read)
            .WillOnce([] {
              google::bigtable::v2::SampleRowKeysResponse resp;
              resp.set_row_key("r2");
              resp.set_offset_bytes(2);
              return resp;
            })
            .WillOnce(Return(Status()));
        return reader;
      });

  internal::OptionsSpan span(TestCallOptions(/*expected_retries=*/1));
  auto connection = TestConnection();
  auto resp = connection->SampleRowKeys({});
  ASSERT_STATUS_OK(resp);
  auto samples = RowKeySampleVectors(*resp);
  EXPECT_THAT(samples.row_keys, ElementsAre("r2"));
  EXPECT_THAT(samples.offset_bytes, ElementsAre(2));
}

TEST_F(BigtableConnectionTest, SampleRowsRetryExhausted) {
  auto constexpr kNumRetries = 2;

  EXPECT_CALL(*mock_stub_, SampleRowKeys)
      .Times(kNumRetries + 1)
      .WillRepeatedly([](std::unique_ptr<grpc::ClientContext>,
                   google::bigtable::v2::SampleRowKeysRequest const&) {
        auto reader = absl::make_unique<
            bigtable::testing::MockSampleRowsStreamingReadRpc>();
        EXPECT_CALL(*reader, Read)
            .WillOnce(Return(Status(StatusCode::kUnavailable, "try again")));
        return reader;
      });

  auto retry = std::make_shared<bigtable::BigtableLimitedErrorCountRetryPolicy>(
      kNumRetries);
  internal::OptionsSpan span(
      TestCallOptions(/*expected_retries=*/kNumRetries)
          .set<bigtable::BigtableRetryPolicyOption>(std::move(retry)));
  auto connection = TestConnection();
  auto resp = connection->SampleRowKeys({});
  EXPECT_THAT(
      resp, StatusIs(StatusCode::kUnavailable,
                     AllOf(HasSubstr("Retry policy exhausted:"), HasSubstr("try again"))));
}

}  // namespace
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google
