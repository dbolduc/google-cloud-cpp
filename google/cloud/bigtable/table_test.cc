// Copyright 2017 Google Inc.
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

#include "google/cloud/bigtable/table.h"
#include "google/cloud/bigtable/mocks/mock_bigtable_connection.h"
#include "google/cloud/bigtable/testing/mock_async_failing_rpc_factory.h"
#include "google/cloud/bigtable/testing/table_test_fixture.h"
#include "google/cloud/internal/background_threads_impl.h"
#include "google/cloud/testing_util/chrono_literals.h"
#include "google/cloud/testing_util/fake_completion_queue_impl.h"
#include "google/cloud/testing_util/status_matchers.h"
#include "google/bigtable/v2/bigtable.pb.h"
#include "row_key_sample.h"

namespace google {
namespace cloud {
namespace bigtable {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace {

namespace btproto = ::google::bigtable::v2;
using ::google::cloud::testing_util::chrono_literals::operator"" _ms;
using ::google::cloud::testing_util::FakeCompletionQueueImpl;
using ::google::cloud::testing_util::IsProtoEqual;
using ::google::cloud::testing_util::StatusIs;
using ::google::protobuf::TextFormat;
using ::testing::AllOf;
using ::testing::ElementsAreArray;
using ::testing::HasSubstr;
using ::testing::Property;
using ::testing::Return;

/// Define types and functions used in the tests.
namespace {
class TableTest : public ::google::cloud::bigtable::testing::TableTestFixture {
 public:
  TableTest() : TableTestFixture(CompletionQueue{}) {}
};
}  // anonymous namespace

TEST_F(TableTest, ClientProjectId) {
  EXPECT_EQ(kProjectId, client_->project_id());
}

TEST_F(TableTest, ClientInstanceId) {
  EXPECT_EQ(kInstanceId, client_->instance_id());
}

TEST_F(TableTest, StandaloneInstanceName) {
  EXPECT_EQ(kInstanceName, InstanceName(client_));
}

TEST_F(TableTest, StandaloneTableName) {
  EXPECT_EQ(kTableName, TableName(client_, kTableId));
}

TEST_F(TableTest, TableName) { EXPECT_EQ(kTableName, table_.table_name()); }

TEST_F(TableTest, WithNewTarget) {
  Table table(client_, "original-profile", kTableId);
  EXPECT_EQ(table.project_id(), kProjectId);
  EXPECT_EQ(table.instance_id(), kInstanceId);
  EXPECT_EQ(table.table_id(), kTableId);
  EXPECT_EQ(table.table_name(), TableName(kProjectId, kInstanceId, kTableId));
  EXPECT_EQ(table.app_profile_id(), "original-profile");

  std::string const other_project_id = "other-project";
  std::string const other_instance_id = "other-instance";
  std::string const other_table_id = "other-table";
  auto other_table =
      table.WithNewTarget(other_project_id, other_instance_id, other_table_id);

  EXPECT_EQ(other_table.project_id(), other_project_id);
  EXPECT_EQ(other_table.instance_id(), other_instance_id);
  EXPECT_EQ(other_table.table_id(), other_table_id);
  EXPECT_EQ(other_table.table_name(),
            TableName(other_project_id, other_instance_id, other_table_id));
  EXPECT_EQ(other_table.app_profile_id(), "original-profile");
}

TEST_F(TableTest, WithNewTargetProfile) {
  Table table(client_, "original-profile", kTableId);
  EXPECT_EQ(table.project_id(), kProjectId);
  EXPECT_EQ(table.instance_id(), kInstanceId);
  EXPECT_EQ(table.table_id(), kTableId);
  EXPECT_EQ(table.table_name(), TableName(kProjectId, kInstanceId, kTableId));
  EXPECT_EQ(table.app_profile_id(), "original-profile");

  std::string const other_project_id = "other-project";
  std::string const other_instance_id = "other-instance";
  std::string const other_table_id = "other-table";
  std::string const other_profile_id = "other-profile";
  auto other_table = table.WithNewTarget(other_project_id, other_instance_id,
                                         other_profile_id, other_table_id);

  EXPECT_EQ(other_table.project_id(), other_project_id);
  EXPECT_EQ(other_table.instance_id(), other_instance_id);
  EXPECT_EQ(other_table.table_id(), other_table_id);
  EXPECT_EQ(other_table.table_name(),
            TableName(other_project_id, other_instance_id, other_table_id));
  EXPECT_EQ(other_table.app_profile_id(), other_profile_id);
}

TEST_F(TableTest, TableConstructor) {
  std::string const other_table_id = "my-table";
  std::string const other_table_name = TableName(client_, other_table_id);
  Table table(client_, other_table_id);
  EXPECT_EQ(other_table_name, table.table_name());
}

TEST_F(TableTest, CopyConstructor) {
  Table source(client_, "my-table");
  std::string const& expected = source.table_name();
  // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
  Table copy(source);
  EXPECT_EQ(expected, copy.table_name());
}

TEST_F(TableTest, MoveConstructor) {
  Table source(client_, "my-table");
  std::string expected = source.table_name();
  Table copy(std::move(source));
  EXPECT_EQ(expected, copy.table_name());
}

TEST_F(TableTest, CopyAssignment) {
  Table source(client_, "my-table");
  std::string const& expected = source.table_name();
  Table dest(client_, "another-table");
  dest = source;
  EXPECT_EQ(expected, dest.table_name());
}

TEST_F(TableTest, MoveAssignment) {
  Table source(client_, "my-table");
  std::string expected = source.table_name();
  Table dest(client_, "another-table");
  dest = std::move(source);
  EXPECT_EQ(expected, dest.table_name());
}

TEST_F(TableTest, ChangeOnePolicy) {
  Table table(client_, "some-table", AlwaysRetryMutationPolicy());
  EXPECT_EQ("", table.app_profile_id());
  EXPECT_THAT(table.table_name(), HasSubstr("some-table"));
}

TEST_F(TableTest, ChangePolicies) {
  Table table(client_, "some-table", AlwaysRetryMutationPolicy(),
              LimitedErrorCountRetryPolicy(42));
  EXPECT_EQ("", table.app_profile_id());
  EXPECT_THAT(table.table_name(), HasSubstr("some-table"));
}

TEST_F(TableTest, ConstructorWithAppProfileAndPolicies) {
  Table table(client_, "test-profile-id", "some-table",
              AlwaysRetryMutationPolicy(), LimitedErrorCountRetryPolicy(42));
  EXPECT_EQ("test-profile-id", table.app_profile_id());
  EXPECT_THAT(table.table_name(), HasSubstr("some-table"));
}

std::string const kProjectId = "the-project";
std::string const kInstanceId = "the-instance";
std::string const kTableId = "the-table";

class ValidContextMdAsyncTest : public ::testing::Test {
 public:
  ValidContextMdAsyncTest()
      : cq_impl_(new FakeCompletionQueueImpl),
        cq_(cq_impl_),
        client_(new ::google::cloud::bigtable::testing::MockDataClient(
            Options{}.set<GrpcCompletionQueueOption>(cq_))) {
    EXPECT_CALL(*client_, project_id())
        .WillRepeatedly(::testing::ReturnRef(kProjectId));
    EXPECT_CALL(*client_, instance_id())
        .WillRepeatedly(::testing::ReturnRef(kInstanceId));
    table_ = absl::make_unique<Table>(client_, kTableId);
  }

 protected:
  template <typename ResultType>
  void FinishTest(
      ::google::cloud::future<google::cloud::StatusOr<ResultType>> res_future) {
    EXPECT_EQ(1U, cq_impl_->size());
    cq_impl_->SimulateCompletion(true);
    EXPECT_EQ(0U, cq_impl_->size());
    auto res = res_future.get();
    EXPECT_FALSE(res);
    EXPECT_EQ(::google::cloud::StatusCode::kPermissionDenied,
              res.status().code());
  }

  void FinishTest(::google::cloud::future<google::cloud::Status> res_future) {
    EXPECT_EQ(1U, cq_impl_->size());
    cq_impl_->SimulateCompletion(true);
    EXPECT_EQ(0U, cq_impl_->size());
    auto res = res_future.get();
    EXPECT_EQ(::google::cloud::StatusCode::kPermissionDenied, res.code());
  }

  std::shared_ptr<FakeCompletionQueueImpl> cq_impl_;
  CompletionQueue cq_;
  std::shared_ptr<testing::MockDataClient> client_;
  std::unique_ptr<Table> table_;
};

TEST_F(ValidContextMdAsyncTest, AsyncApply) {
  testing::MockAsyncFailingRpcFactory<btproto::MutateRowRequest,
                                      btproto::MutateRowResponse>
      rpc_factory;
  EXPECT_CALL(*client_, AsyncMutateRow)
      .WillOnce(rpc_factory.Create(
          R"""(
              table_name: "projects/the-project/instances/the-instance/tables/the-table"
              row_key: "row_key"
              mutations: { delete_from_row { } }
          )""",
          "google.bigtable.v2.Bigtable.MutateRow"));
  FinishTest(table_->AsyncApply(SingleRowMutation("row_key", DeleteFromRow())));
}

TEST_F(ValidContextMdAsyncTest, AsyncCheckAndMutateRow) {
  testing::MockAsyncFailingRpcFactory<btproto::CheckAndMutateRowRequest,
                                      btproto::CheckAndMutateRowResponse>
      rpc_factory;
  EXPECT_CALL(*client_, AsyncCheckAndMutateRow)
      .WillOnce(rpc_factory.Create(
          R"""(
              table_name: "projects/the-project/instances/the-instance/tables/the-table"
              row_key: "row_key"
              true_mutations: { delete_from_row { } }
              predicate_filter: { pass_all_filter: true }
          )""",
          "google.bigtable.v2.Bigtable.CheckAndMutateRow"));
  FinishTest(table_->AsyncCheckAndMutateRow("row_key", Filter::PassAllFilter(),
                                            {DeleteFromRow()}, {}));
}

TEST_F(ValidContextMdAsyncTest, AsyncReadModifyWriteRow) {
  testing::MockAsyncFailingRpcFactory<btproto::ReadModifyWriteRowRequest,
                                      btproto::ReadModifyWriteRowResponse>
      rpc_factory;
  EXPECT_CALL(*client_, AsyncReadModifyWriteRow)
      .WillOnce(rpc_factory.Create(
          R"""(
              table_name: "projects/the-project/instances/the-instance/tables/the-table"
              row_key: "row_key"
              rules: {
                  family_name: "fam"
                  column_qualifier: "counter"
                  increment_amount: 1
              }
              rules: {
                  family_name: "fam"
                  column_qualifier: "list"
                  append_value: ";element"
              }
          )""",
          "google.bigtable.v2.Bigtable.ReadModifyWriteRow"));
  FinishTest(table_->AsyncReadModifyWriteRow(
      "row_key", ReadModifyWriteRule::IncrementAmount("fam", "counter", 1),
      ReadModifyWriteRule::AppendValue("fam", "list", ";element")));
}

// TODO : Might as well add the unit tests now.
Status FailingStatus() { return Status(StatusCode::kPermissionDenied, "fail"); }

TEST_F(TableTest, CheckAndMutateRowSuccess) {
  auto mock = std::make_shared<bigtable_mocks::MockBigtableConnection>();
  table_.set_connection(mock);

  std::string expected_raw = R"pb(
    table_name: 'projects/foo-project/instances/bar-instance/tables/baz-table'
    row_key: 'row-key'
    predicate_filter { pass_all_filter: true }
    true_mutations {
      set_cell { family_name: 'f1' column_qualifier: 'c1' value: 'true1' }
    }
    true_mutations {
      set_cell { family_name: 'f2' column_qualifier: 'c2' value: 'true2' }
    }
    false_mutations {
      set_cell { family_name: 'f1' column_qualifier: 'c1' value: 'false1' }
    }
    false_mutations {
      set_cell { family_name: 'f2' column_qualifier: 'c2' value: 'false2' }
    }
  )pb";
  btproto::CheckAndMutateRowRequest expected;
  EXPECT_TRUE(TextFormat::ParseFromString(expected_raw, &expected));

  EXPECT_CALL(*mock, CheckAndMutateRow)
      .WillOnce([&expected](btproto::CheckAndMutateRowRequest const& request) {
        // TODO : Check Policies
        // CheckPolicies(google::cloud::internal::CurrentOptions());
        EXPECT_THAT(expected, IsProtoEqual(request));
        btproto::CheckAndMutateRowResponse resp;
        resp.set_predicate_matched(false);
        return resp;
      })
      .WillOnce([&expected](btproto::CheckAndMutateRowRequest const& request) {
        // TODO : Check Policies
        // CheckPolicies(google::cloud::internal::CurrentOptions());
        EXPECT_THAT(expected, IsProtoEqual(request));
        btproto::CheckAndMutateRowResponse resp;
        resp.set_predicate_matched(true);
        return resp;
      });

  std::vector<Mutation> true_mutations = {
      bigtable::SetCell("f1", "c1", 0_ms, "true1"),
      bigtable::SetCell("f2", "c2", 0_ms, "true2")};

  std::vector<Mutation> false_mutations = {
      bigtable::SetCell("f1", "c1", 0_ms, "false1"),
      bigtable::SetCell("f2", "c2", 0_ms, "false2")};

  auto predicate = table_.CheckAndMutateRow("row-key", Filter::PassAllFilter(),
                                            true_mutations, false_mutations);
  ASSERT_STATUS_OK(predicate);
  EXPECT_EQ(*predicate, MutationBranch::kPredicateNotMatched);

  predicate = table_.CheckAndMutateRow("row-key", Filter::PassAllFilter(),
                                       true_mutations, false_mutations);
  ASSERT_STATUS_OK(predicate);
  EXPECT_EQ(*predicate, MutationBranch::kPredicateMatched);
}

TEST_F(TableTest, CheckAndMutateRowFailure) {
  auto mock = std::make_shared<bigtable_mocks::MockBigtableConnection>();
  table_.set_connection(mock);

  EXPECT_CALL(*mock, CheckAndMutateRow).WillOnce(Return(FailingStatus()));

  auto predicate =
      table_.CheckAndMutateRow("row-key", Filter::PassAllFilter(), {}, {});
  EXPECT_THAT(predicate, StatusIs(StatusCode::kPermissionDenied));
}

TEST_F(TableTest, SampleRowsSuccess) {
  auto mock = std::make_shared<bigtable_mocks::MockBigtableConnection>();
  table_.set_connection(mock);

  EXPECT_CALL(*mock, SampleRowKeys)
      .WillOnce([](btproto::SampleRowKeysRequest const& request) {
        // TODO : Check Policies
        // CheckPolicies(google::cloud::internal::CurrentOptions());
        EXPECT_EQ(kTableName, request.table_name());
        // TODO : Check AppProfile ID?
        std::vector<btproto::SampleRowKeysResponse> resp(2);
        resp[0].set_row_key("r1");
        resp[0].set_offset_bytes(1);
        resp[1].set_row_key("r2");
        resp[1].set_offset_bytes(2);
        return resp;
      });

  // TODO : probably a nicer way to write this. Defining a RowKeySample matcher.
  auto samples = table_.SampleRows();
  ASSERT_STATUS_OK(samples);
  auto it = samples->begin();
  EXPECT_NE(it, samples->end());
  EXPECT_EQ(it->row_key, "r1");
  EXPECT_EQ(it->offset_bytes, 1);
  EXPECT_NE(++it, samples->end());
  EXPECT_EQ(it->row_key, "r2");
  EXPECT_EQ(it->offset_bytes, 2);
  EXPECT_EQ(++it, samples->end());
}

TEST_F(TableTest, SampleRowsFailure) {
  auto mock = std::make_shared<bigtable_mocks::MockBigtableConnection>();
  table_.set_connection(mock);

  EXPECT_CALL(*mock, SampleRowKeys).WillOnce(Return(FailingStatus()));

  auto resp = table_.SampleRows();
  EXPECT_THAT(resp, StatusIs(StatusCode::kPermissionDenied));
}

}  // namespace
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable
}  // namespace cloud
}  // namespace google
