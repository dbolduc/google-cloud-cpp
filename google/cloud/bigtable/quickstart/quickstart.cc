// Copyright 2023 Google LLC
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

#include "google/cloud/bigtable/admin/bigtable_table_admin_client.h"
#include "google/cloud/bigtable/mutation_batcher.h"
#include "google/cloud/bigtable/table.h"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <ratio>
#include <sstream>
#include <string>

namespace {

// Create namespace aliases to make the code easier to read.
namespace gc = ::google::cloud;
namespace cbt = ::google::cloud::bigtable;
namespace cbta = ::google::cloud::bigtable_admin;
using clock = std::chrono::steady_clock;

struct Configuration {
  std::string project_id;
  std::string instance_id;
  std::string table_id;

  // The endpoint to use for requests.
  std::string data_endpoint = "bigtable.googleapis.com";
  // TODO : set GrpcNumChannelOption?

  // The total rows in the table.
  std::int64_t total_rows = 420000;
  // The number of disjoint row ranges.
  std::int64_t disjoint_row_ranges = 1000;
  // The amount of rows we expect to read from each disjoint row range.
  std::int64_t rows_per_range = 60;

  // If true, create a table, and initialize its data.
  bool create_table = false;
  std::int64_t initial_splits = 1000;
  std::string column_family = "family";
  std::string column = "column";
  std::int64_t bytes_per_row = 2 * 1024;  // 2 KiB

  // If true, delete the table at the end of execution. Note that tables may
  // leak if execution is stopped abruptly.
  bool delete_table = false;
};

// Fill the index with leading 0s, so that the row keys are a fixed width.
std::string MakeRowString(int key_width, int64_t row_index) {
  std::ostringstream os;
  os << "row" << std::setw(key_width) << std::setfill('0') << row_index;
  return std::move(os).str();
}

// Compute ceil(log_10(kTotalRows))
int RowKeyWidth(std::int64_t total_rows) {
  int width = 0;
  while (total_rows != 0) {
    total_rows /= 10;
    ++width;
  }
  return width;
}

void ConditionallyCreateTable(Configuration const& config);
void ConditionallyDeleteTable(Configuration const& config);

}

int main(int argc, char* argv[]) try {
  if (argc != 4) {
    std::cerr << "Usage: quickstart <project-id> <instance-id> <table-id>\n";
    return 1;
  }

  Configuration config;
  config.project_id = argv[1];
  config.instance_id = argv[2];
  config.table_id = argv[3];

  std::cout << "\nCONFIGURATION:"
            << "\nTotal Rows: " << config.total_rows
            << "\nDisjoint Sets: " << config.disjoint_row_ranges
            << "\nRows Per Range: " << config.rows_per_range
            << "\n\nSETUP:" << std::endl;

  ConditionallyCreateTable(config);

  auto const row_key_width = RowKeyWidth(config.total_rows);
  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);

  auto options = gc::Options{}.set<gc::EndpointOption>(config.data_endpoint);
  auto table = cbt::Table(cbt::MakeDataConnection(options), tr);

  std::cout << "Start sampling row keys." << std::endl;
  auto samples_sor = table.SampleRows();
  if (!samples_sor) throw std::move(samples_sor).status();
  auto samples = *std::move(samples_sor);
  std::cout << "Finished sampling row keys." << std::endl;

  // The row key samples tell us the boundary of each split of our table.
  std::vector<cbt::RowSet> row_sets;
  row_sets.reserve(samples.size());
  // TODO : document. Also consider map<int, RowSet> -> vector<RowSet>
  int current_split = -1;
  std::cout << "Start reading disjoint ranges." << std::endl;
  auto split = 0;
  for (auto i = 0; i != config.disjoint_row_ranges; ++i){
    auto start_index = i * config.total_rows / config.disjoint_row_ranges;
    auto start_row_key = MakeRowString(row_key_width, start_index);
    // Instead of subtracing one, we could use a RightOpen RowRange.
    auto end_index = start_index + config.rows_per_range - 1;
    auto end_row_key = MakeRowString(row_key_width, end_index);

    // Bigtable returns "" to mean "end-of-table".
    //
    // `std::string{"row"} > std::string{""}`, so we must handle the empty
    // string case separately.
    //
    // Returns true if r1 >= r2. Also, we can assume that r1 != "".
    auto row_key_ge = [](std::string const& r1, std::string const& r2) {
      return !r2.empty() && r1 >= r2;
    };
    while (row_key_ge(start_row_key, samples[split].row_key)) ++split;
    while (row_key_ge(end_row_key, samples[split].row_key)) {
      // TODO : check_if_new_split();
      if (current_split != split) {
        row_sets.emplace_back();
        current_split = split;
      }
      row_sets.back().Append(cbt::RowRange::RightOpen(std::move(start_row_key),
                                                      samples[split].row_key));
      start_row_key = samples[split].row_key;
      ++split;
    }
    if (current_split != split) {
      row_sets.emplace_back();
      current_split = split;
    }
    row_sets.back().Append(cbt::RowRange::Closed(std::move(start_row_key),
                                                 std::move(end_row_key)));
  }
  std::cout << "Finished reading disjoint ranges." << std::endl;

  struct SplitResult {
    // Counts the rows read per split.
    int rows;
    // TODO : a more expensive accumulation.
    std::vector<std::string> row_keys;
    // Lets us block until the operation has completed.
    gc::promise<gc::Status> promise;
  };
  std::vector<SplitResult> accumulators(row_sets.size());

  // Start a timer.
  auto start_time = clock::now();

  std::cout << "\nREAD ROWS:" << "\n";
  for (std::size_t i = 0; i != row_sets.size(); ++i) {
    auto& accumulator = accumulators[i];
    auto on_row = [&accumulator](cbt::Row const& row) {
      ++accumulator.rows;
      // TODO : Note that this copies the row key.
      accumulator.row_keys.emplace_back(row.row_key());
      return gc::make_ready_future(true);
    };
    auto on_finish = [&accumulator](gc::Status status) {
      accumulator.promise.set_value(std::move(status));
    };
    table.AsyncReadRows(on_row, on_finish, std::move(row_sets[i]),
                        cbt::Filter::Latest(1));
    std::cout << "Reading for split: " << i << "\n";
  }

  std::cout << "Accumulating...\n";
  auto total_rows = 0;
  for (std::size_t i = 0; i != row_sets.size(); ++i) {
    auto f = accumulators[i].promise.get_future();
    auto status = f.get();
    if (!status.ok()) throw std::move(status);
    total_rows += accumulators[i].rows;
    std::cout << "Accumulator[" << i << "].rows = " << accumulators[i].rows
              << "\n";
  }

  // End the timer.
  auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - start_time);

  // Report results.
  std::cout << "\nRESULTS:" << std::endl;
  std::cout << "Elapsed time (milliseconds): " << elapsed.count() << std::endl;
  std::cout << "Rows read: " << total_rows << std::endl;

  ConditionallyDeleteTable(config);

  // TODO : consider showing the synchronous code.

  return 0;
} catch (gc::Status const& status) {
  std::cerr << "google::cloud::Status thrown: " << status << "\n";
  return 1;
}

namespace {

void WriteTableData(Configuration const& config) {
    if (!config.create_table) return;

    auto const row_key_width = RowKeyWidth(config.total_rows);
    auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                       config.table_id);
    auto const value = std::string(config.bytes_per_row, '0');

    auto options = gc::Options{}.set<gc::EndpointOption>(config.data_endpoint);
    auto table = cbt::Table(cbt::MakeDataConnection(options), tr);

    std::cout << "Start writing data." << std::endl;

    // Initialize the data in the table.
    cbt::MutationBatcher batcher(table);
    gc::CompletionQueue cq;
    std::thread cq_runner([&cq]() { cq.Run(); });

    for (std::int64_t i = 0; i != config.total_rows; ++i) {
    auto row_key = MakeRowString(row_key_width, i);
    auto mut = cbt::SingleRowMutation(
        std::move(row_key),
        {cbt::SetCell(config.column_family, config.column, value)});
    auto admission_completion = batcher.AsyncApply(cq, std::move(mut));
    auto& admission_future = admission_completion.first;
    auto& completion_future = admission_completion.second;
    completion_future.then([](auto completion_status) {
      auto s = completion_status.get();
      if (!s.ok()) throw std::move(s);
    });
    admission_future.get();
    }
    // Wait for all mutations to complete
    batcher.AsyncWaitForNoPendingRequests().get();
    cq.Shutdown();
    cq_runner.join();

    std::cout << "Finished writing data." << std::endl;
}

void ConditionallyCreateTable(Configuration const& config) {
  if (!config.create_table) return;

  auto const row_key_width = RowKeyWidth(config.total_rows);
  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);
  auto admin =
      cbta::BigtableTableAdminClient(cbta::MakeBigtableTableAdminConnection());

  google::bigtable::admin::v2::CreateTableRequest r;
  r.set_parent(tr.instance().FullName());
  r.set_table_id(tr.table_id());
  // Provide initial splits to the table
  for (auto i = 1; i < config.initial_splits; ++i) {
    auto row_index = i * config.total_rows / config.initial_splits;
    r.add_initial_splits()->set_key(MakeRowString(row_key_width, row_index));
    }
    auto& families = *r.mutable_table()->mutable_column_families();
    families[config.column_family].mutable_gc_rule()->set_max_num_versions(1);

    std::cout << "Start creating table." << std::endl;
    auto status = admin.CreateTable(r);
    if (!status.ok()) throw std::move(status).status();
    std::cout << "Finished creating table." << std::endl;

    WriteTableData(config);
}

void ConditionallyDeleteTable(Configuration const& config) {
  if (!config.delete_table) return;

  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);
  std::cout << "Start deleting table." << std::endl;
  auto admin =
      cbta::BigtableTableAdminClient(cbta::MakeBigtableTableAdminConnection());
  auto status = admin.DeleteTable(tr.FullName());
  if (!status.ok()) throw std::move(status);
  std::cout << "Finished deleting table." << std::endl;
}

}  // namespace
