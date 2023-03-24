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

auto constexpr kUsage =
    "Usage: quickstart <project-id> <instance-id> <table-id>\n";

auto constexpr kDescription =
    R"""(An example application for reading disjoint row ranges.

The program will:

 1) Conditionally create a table, with initial splits.
 2) Echo your configuration settings.
 3) Sample the row keys to determine the splits of the table.
 4) Generate disjoint row ranges.
 5) Group the row ranges into buckets, as determined by the splits of the table.
 6) Start a timer.
 7) Asynchronously read the rows in a bucket's row set, for each bucket.
 8) Accumulate the number of rows seen, for each bucket.
 9) Block until all rows have been read.
10) Sum the total rows read across the buckets.
11) Stop the timer.
11) Report the elapsed time of the reads, and the number of rows read.
12) Conditionally delete the table.

For example, if the program is configured with
- 5000 total_rows
- 10 disjoint_row_ranges
- 5 rows_per_range
- 4 initial_splits (and create_table == true)

It will create a table with rows from "row0000" to "row5000". The table will
have splits: ["row1250", "row2500", "row3750", ""]. It will generate the row
ranges [["row0000", "row0004"], ["row0500", "row0504"], ..., ["row4500", "row4504"]].
)""";

// Create namespace aliases to make the code easier to read.
namespace gc = ::google::cloud;
namespace cbt = ::google::cloud::bigtable;
namespace cbta = ::google::cloud::bigtable_admin;
using clock = std::chrono::steady_clock;

struct Configuration {
  std::string project_id;
  std::string instance_id;
  std::string table_id;

  // The endpoint to use for data requests (`cbt::Table`).
  std::string data_endpoint = "bigtable.googleapis.com";
  // The number of channels used in the `cbt::DataConnection`. Any value greater
  // than the `initial_splits` of the table is gratuitous.
  int grpc_num_channels = 10;

  // The total rows in the table.
  std::int64_t total_rows = 420000;
  // The number of disjoint row ranges.
  std::int64_t disjoint_row_ranges = 1000;
  // The amount of rows we expect to read from each disjoint row range.
  std::int64_t rows_per_range = 60;

  // If true, create a table, and initialize its data.
  bool create_table = false;
  std::int64_t initial_splits = 10;
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

// Compute ceil(log_10(total_rows))
int RowKeyWidth(std::int64_t total_rows) {
  int width = 0;
  while (total_rows != 0) {
    total_rows /= 10;
    ++width;
  }
  return width;
}

// Returns true if `row` >= `split`...
//
// But there is a catch. Bigtable returns "" to mean "end-of-table".
// `std::string{"row"} > std::string{""}`, but we want "end-of-table" to be
// greater than all rows. So we need special handling for the case where `split`
// is empty. Aside: note that `row` cannot be empty.
bool IsRowPastSplit(std::string const& row, std::string const& split) {
  return !split.empty() && row >= split;
};

// A helper class to group row ranges into buckets, given the splits of a table.
class Buckets {
 public:
  explicit Buckets(std::vector<cbt::RowKeySample> const& samples)
      : samples_(samples), split_(samples_.begin()) {
    row_sets_.reserve(samples.size());
  }

  // Assumes that row keys are (1) non-overlapping, (2) entered in order.
  void AddClosedRange(std::string start_row_key, std::string end_row_key) {
    // The range is past the current split.
    while (IsRowPastSplit(start_row_key, split_->row_key)) Inc();

    // The range straddles at least one split.
    while (IsRowPastSplit(end_row_key, split_->row_key)) {
      Append(
          cbt::RowRange::RightOpen(std::move(start_row_key), split_->row_key));
      start_row_key = split_->row_key;
      Inc();
    }

    // The range is contained within the current split.
    Append(cbt::RowRange::Closed(std::move(start_row_key),
                                 std::move(end_row_key)));
  }

  // Returns the collected row sets and invalidates the object.
  std::vector<cbt::RowSet> RowSets() && { return std::move(row_sets_); }

 private:
  // Look to the next split.
  void Inc() {
    ++split_;
    new_split_ = true;
  }

  void Append(cbt::RowRange range) {
    // Prepare a new `RowSet` if we crossed a split of the table. This makes it
    // easier to skip over splits which do not overlap with any of our disjoint
    // row ranges.
    //
    // We do this because a default constructed `RowSet` means read *all* rows.
    if (new_split_) {
      row_sets_.emplace_back();
      new_split_ = false;
    }
    row_sets_.back().Append(std::move(range));
  }

  std::vector<cbt::RowSet> row_sets_;
  // The row key samples tell us the boundary of each split of our table. We
  // will group row ranges into buckets according to these boundaries.
  std::vector<cbt::RowKeySample> const& samples_;
  std::vector<cbt::RowKeySample>::const_iterator split_;
  bool new_split_ = true;
};

void ConditionallyCreateTable(Configuration const& config);
void ConditionallyDeleteTable(Configuration const& config);

}  // namespace

int main(int argc, char* argv[]) try {
  if (argc == 2 && argv[1] == std::string{"--description"}) {
    std::cout << kDescription;
    return 0;
  }
  if (argc == 2 && argv[1] == std::string{"--help"}) {
    std::cout << kUsage;
    return 0;
  }
  if (argc != 4) {
    std::cerr << kUsage;
    return 1;
  }

  Configuration config;
  config.project_id = argv[1];
  config.instance_id = argv[2];
  config.table_id = argv[3];

  std::cout << "\nCONFIGURATION:"
            << "\nTotal Rows: " << config.total_rows
            << "\nDisjoint Sets: " << config.disjoint_row_ranges
            << "\nRows Per Range: " << config.rows_per_range << "\n\nSETUP:";

  ConditionallyCreateTable(config);

  auto const row_key_width = RowKeyWidth(config.total_rows);
  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);

  // Make a Table client to connect to the Bigtable Data API.
  auto options = gc::Options{}
                     .set<gc::EndpointOption>(config.data_endpoint)
                     .set<gc::GrpcNumChannelsOption>(config.grpc_num_channels);
  auto table = cbt::Table(cbt::MakeDataConnection(options), tr);

  std::cout << "\nStart sampling row keys.";
  auto samples_sor = table.SampleRows();
  if (!samples_sor) throw std::move(samples_sor).status();
  auto samples = *std::move(samples_sor);
  std::cout << "\nFinished sampling row keys.";

  std::cout << "\nStart reading disjoint ranges.";
  Buckets buckets(samples);
  for (auto i = 0; i != config.disjoint_row_ranges; ++i) {
    auto start_index = i * config.total_rows / config.disjoint_row_ranges;
    auto start_row_key = MakeRowString(row_key_width, start_index);
    // Instead of subtracting one, we could add RightOpenRanges. :shrug:
    auto end_index = start_index + config.rows_per_range - 1;
    auto end_row_key = MakeRowString(row_key_width, end_index);

    buckets.AddClosedRange(std::move(start_row_key), std::move(end_row_key));
  }
  auto row_sets = std::move(buckets).RowSets();
  std::cout << "\nFinished reading disjoint ranges.";

  struct SplitResult {
    // Counts the rows read per split.
    std::int64_t rows;
    // Lets us block until the operation has completed.
    gc::promise<gc::Status> promise;
  };
  std::vector<SplitResult> accumulators(row_sets.size());

  std::cout << "\n\nStart timing.";
  auto start_time = clock::now();

  std::cout << "\nREAD ROWS:";
  for (std::size_t i = 0; i != row_sets.size(); ++i) {
    auto& accumulator = accumulators[i];
    // Increment the row count for this split when we receive a row.
    auto on_row = [&accumulator](cbt::Row const& /*row*/) {
      ++accumulator.rows;
      return gc::make_ready_future(true);
    };
    // Mark the read of this row set as complete when the stream finishes.
    auto on_finish = [&accumulator](gc::Status status) {
      accumulator.promise.set_value(std::move(status));
    };
    table.AsyncReadRows(on_row, on_finish, std::move(row_sets[i]),
                        cbt::Filter::Latest(1));
    std::cout << "\nReading for split: " << i;
  }

  std::cout << "\nAccumulating...";
  auto total_rows = 0;
  for (std::size_t i = 0; i != row_sets.size(); ++i) {
    auto f = accumulators[i].promise.get_future();
    // Block until the read for this row set is complete.
    auto status = f.get();
    if (!status.ok()) throw std::move(status);
    total_rows += accumulators[i].rows;
    std::cout << "\nAccumulator[" << i << "].rows = " << accumulators[i].rows;
  }

  // End the timer.
  auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - start_time);
  std::cout << "\nFinished timing.";

  // Report results.
  std::cout << "\n\nRESULTS:"
            << "\nElapsed time (milliseconds): " << elapsed.count()
            << "\nRows read: " << total_rows << "\n";

  ConditionallyDeleteTable(config);

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

  auto options = gc::Options{}
                     .set<gc::EndpointOption>(config.data_endpoint)
                     .set<gc::GrpcNumChannelsOption>(config.grpc_num_channels);
  auto table = cbt::Table(cbt::MakeDataConnection(options), tr);

  cbt::MutationBatcher batcher(table);
  gc::CompletionQueue cq;
  std::thread cq_runner([&cq]() { cq.Run(); });

  std::cout << "\nStart writing data.";
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
  std::cout << "\nFinished writing data.";

  cq.Shutdown();
  cq_runner.join();
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

  std::cout << "\nStart creating table.";
  auto status = admin.CreateTable(r);
  if (!status.ok()) throw std::move(status).status();
  std::cout << "\nFinished creating table.";

  WriteTableData(config);
}

void ConditionallyDeleteTable(Configuration const& config) {
  if (!config.delete_table) return;

  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);
  std::cout << "\nStart deleting table.";
  auto admin =
      cbta::BigtableTableAdminClient(cbta::MakeBigtableTableAdminConnection());
  auto status = admin.DeleteTable(tr.FullName());
  if (!status.ok()) throw std::move(status);
  std::cout << "\nFinished deleting table.";
}

}  // namespace
