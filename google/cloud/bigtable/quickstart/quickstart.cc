// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// [START bigtable_quickstart]
#include "google/cloud/bigtable/admin/bigtable_table_admin_client.h"
#include "google/cloud/bigtable/resource_names.h"

/*
 * Run quickstart with OT:
 * bazel run //google/cloud/bigtable/quickstart:quickstart --//:open_telemetry=True
 *
 * Run quickstart without OT:
 * bazel run //google/cloud/bigtable/quickstart:quickstart
 */

int main() {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
  std::cout << "GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY defined in quickstart.\n";
#endif //  GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

  std::string const project_id = "dbolduc-test";
  std::string const instance_id = "test-instance";

  namespace cbt = ::google::cloud::bigtable;
  namespace cbta = ::google::cloud::bigtable_admin;

  auto conn = cbta::MakeBigtableTableAdminConnection();
  auto client = cbta::BigtableTableAdminClient(std::move(conn));

  google::bigtable::admin::v2::ListTablesRequest list_req;
  list_req.set_parent(cbt::InstanceName(project_id, instance_id));
  auto tables = client.ListTables(list_req);
  for (auto& t : tables) {
    if (!t) {
     std::cout << std::move(t).status() << "\n";
     return 1;
    }
    std::cout << "Table: " << t->DebugString() << "\n";
  }

  return 0;
}
// [END bigtable_quickstart]
