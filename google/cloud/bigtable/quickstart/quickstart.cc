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

//! [START bigtable_quickstart] [all]
#include "google/cloud/bigtable/table.h"

int main(int argc, char* argv[]) try {
  if (argc != 5) {
    std::string const cmd = argv[0];
    auto last_slash = std::string(cmd).find_last_of('/');
    std::cerr << "Usage: " << cmd.substr(last_slash + 1)
              << " <project_id> <instance_id> <table_id> <app_profile_id>\n";
    return 1;
  }

  std::string const project_id = argv[1];
  std::string const instance_id = argv[2];
  std::string const table_id = argv[3];
  std::string const profile_id = argv[4];

  // Create a namespace alias to make the code easier to read.
  namespace gc = ::google::cloud;
  namespace cbt = ::google::cloud::bigtable;
  auto options = gc::Options{}.set<cbt::AppProfileIdOption>(profile_id);
  cbt::Table table(cbt::MakeDataConnection(options),
                   cbt::TableResource(project_id, instance_id, table_id));

  std::string row_key = "r1";
  std::string column_family = "fam";

  cbt::SingleRowMutation mut(
      "r1", cbt::SetCell("fam", "cq", std::chrono::milliseconds(0), "value"));
  auto status = table.Apply(mut);
  if (!status.ok()) throw std::move(status);
  std::cout << "Success?" << std::endl;
  return 0;

  std::cout << "Getting a single row by row key:" << std::flush;
  google::cloud::StatusOr<std::pair<bool, cbt::Row>> result =
      table.ReadRow(row_key, cbt::Filter::FamilyRegex(column_family));
  if (!result) throw std::move(result).status();
  if (!result->first) {
    std::cout << "Cannot find row " << row_key << " in the table: " << table_id
              << "\n";
    return 0;
  }

  cbt::Cell const& cell = result->second.cells().front();
  std::cout << cell.family_name() << ":" << cell.column_qualifier() << "    @ "
            << cell.timestamp().count() << "us\n"
            << '"' << cell.value() << '"' << "\n";

  return 0;
} catch (google::cloud::Status const& status) {
  std::cerr << "google::cloud::Status thrown: " << status << "\n";
  return 1;
}
//! [END bigtable_quickstart] [all]
