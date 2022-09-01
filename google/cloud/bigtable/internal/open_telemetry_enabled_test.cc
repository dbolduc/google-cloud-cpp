// Copyright 2021 Google LLC
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

#include "google/cloud/bigtable/admin/internal/bigtable_table_admin_open_telemetry.h"
#include "google/cloud/bigtable/admin/mocks/mock_bigtable_table_admin_connection.h"
#include "google/cloud/testing_util/scoped_log.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace bigtable_admin_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace {

using ::google::cloud::testing_util::ScopedLog;
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::Return;

// The point of this test is just to "prove" whether OT is enabled or not. It is
// not something we will want in the long run. It is just for developing.
//
// Basically, we should expect the `asan-ot`, `clang-tidy-ot`, and
// `cmake-install-ot` builds to pass. And their non-ot counterparts to fail.
TEST(OpenTelemetry, Enabled) {
  ScopedLog log;

  auto mock = std::make_shared<
      bigtable_admin_mocks::MockBigtableTableAdminConnection>();
  EXPECT_CALL(*mock, GetTable)
      .WillOnce(Return(Status(StatusCode::kPermissionDenied, "fail")));

  auto conn = MaybeMakeBigtableTableAdminTracingConnection(mock);
  (void)conn->GetTable({});

  EXPECT_THAT(log.ExtractLines(),
              Contains(HasSubstr("TracingConnection::GetTable")));
}

}  // namespace
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google
