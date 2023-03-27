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
#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <random>
#include <ratio>
#include <sstream>
#include <string>

namespace {

auto constexpr kUsage =
    "Usage: quickstart <project-id> <instance-id> <table-id>\n";

auto constexpr kDescription =
    R"""(An example application for reading disjoint row ranges.

This example showcases how to get the most read performance out of Cloud
Bigtable. A Bigtable table is split across nodes within a cluster, and across
tablet servers within a node. We can query the boundaries between tablets with
`Table::SampleRows()`. We can split up a read rows request into buckets for each
tablet. We can then make those requests concurrently with
`Table::AsyncReadRows(...)`.

For more details on how Bigtable stores data, see:
https://cloud.google.com/bigtable/docs/performance#distributing-data

The program will:

1) Conditionally create a table, with initial splits.
2) Echo your configuration settings.
3) Sample the row keys to determine the splits of the table.
4) Generate disjoint row ranges, or read row ranges from a file.
5) Group the row ranges into buckets, as determined by the splits of the table.
6) Run the following read loop N times:
  a) Start a timer.
  b) Asynchronously read the rows in a bucket's row set, for each bucket.
  c) Accumulate the number of rows seen, for each bucket.
  d) Block until all rows have been read.
  e) Sum the total rows read across the buckets.
  f) Stop the timer.
  g) Report the elapsed time of the reads, and the number of rows read.
7) Conditionally delete the table.

For example, if the program is configured with
- 5000 total_rows
- 10 disjoint_row_ranges
- 5 rows_per_range
- 4 initial_splits (and create_table == true)

It will create a table with rows from "row0000" to "row5000". The table will
have splits: ["row1250", "row2500", "row3750", ""]. It will generate the row
ranges [["row0000", "row0004"], ["row0500", "row0504"], ..., ["row4500", "row4504"]].

Additionally, the program will not try very hard to handle errors. It will just
stop execution, and print the error when one is encountered.
)""";

// Create namespace aliases to make the code easier to read.
namespace gc = ::google::cloud;
namespace cbt = ::google::cloud::bigtable;
namespace cbta = ::google::cloud::bigtable_admin;
using clock = std::chrono::steady_clock;

// The parameters of this sample can be configured by editing these values and
// recompiling. Note that the project, instance, and table IDs are supplied via
// the command line.
struct Configuration {
  std::string project_id;
  std::string instance_id;
  std::string table_id;

  // The endpoint to use for data requests (`cbt::Table`).
  std::string data_endpoint = "bigtable.googleapis.com";
  // The number of channels used in the `cbt::DataConnection`. Any value greater
  // than the `initial_splits` of the table is gratuitous.
  int grpc_num_channels = 10;
  // The number of threads servicing the async I/O queue. These are created in
  // the background by the client.
  int grpc_num_threads = 5;
  // The file to read row ranges from. If empty, the application will generate
  // default row ranges for test.
  //
  // See the `MakeRowRangeSource(...)` implementation for more details.
  std::string row_ranges_filepath = "example_ranges.txt";
  // The number of iterations of the experiment to perform. Note that only the
  // read is timed. The application does not re-sample row keys.
  int num_iterations = 5;
  // If true, the sample will log human readable output that explains what it is
  // doing. If false, the sample will log experimental results in csv format.
  bool debug_log = true;

  // The following configuration settings only apply when running this sample
  // with default test data. If you are running this sample application against
  // a custom table, they can be ignored.

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
  // The endpoint to use for table admin requests
  // (`cbta::BigtableTableAdminClient`).
  std::string admin_endpoint = "bigtableadmin.googleapis.com";

  // If true, delete the table at the end of execution. Note that tables may
  // leak if execution is stopped abruptly.
  bool delete_table = false;
};

// Run one iteration of the read loop.
void TimedReadRows(cbt::Table& table, std::vector<cbt::RowSet> row_sets,
                   Configuration const& config) {
  struct BucketReadResult {
    // Counts the rows read per bucket.
    std::int64_t rows;
    // Lets us block until the operation has completed.
    gc::promise<gc::Status> promise;
  };
  std::vector<BucketReadResult> results(row_sets.size());

  if (config.debug_log) std::cout << "Start timing.\n";
  auto start_time = clock::now();

  if (config.debug_log) std::cout << "READ ROWS:\n";
  // Concurrently perform reads for the row sets in each bucket.
  for (std::size_t i = 0; i != row_sets.size(); ++i) {
    auto& result = results[i];

    // Increment the row count for this bucket when we receive a row.
    auto on_row = [&result](cbt::Row const& /*row*/) {
      // This is where you would do something with `row`. In this sample, we
      // just increment a counter.
      ++result.rows;
      return gc::make_ready_future(true);
    };

    // Mark the read of this row set as complete when the stream finishes.
    auto on_finish = [&result](gc::Status status) {
      result.promise.set_value(std::move(status));
    };

    // Call the streaming read RPC
    table.AsyncReadRows(on_row, on_finish, std::move(row_sets[i]),
                        cbt::Filter::Latest(1));
    if (config.debug_log) std::cout << "Reading for bucket: " << i << "\n";
  }

  if (config.debug_log) std::cout << "Waiting...\n";
  auto total_rows = 0;
  for (std::size_t i = 0; i != row_sets.size(); ++i) {
    auto f = results[i].promise.get_future();
    // Block until the read for this row set is complete.
    auto status = f.get();
    if (!status.ok()) throw std::move(status);
    total_rows += results[i].rows;
    if (config.debug_log) {
      std::cout << "Result[" << i << "].rows = " << results[i].rows << "\n";
    }
  }

  // End the timer.
  auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - start_time);
  if (config.debug_log) std::cout << "Finished timing.\n";

  // Report results.
  if (config.debug_log) {
    std::cout << "RESULTS:\n"
              << "Elapsed time (milliseconds): " << elapsed.count() << "\n"
              << "Rows read: " << total_rows << "\n";
  } else {
    std::cout << elapsed.count() << "," << total_rows << ","
              << config.grpc_num_channels << "," << config.grpc_num_threads
              << "," << results.size() << "\n";
  }
}

/**
 * An extension of the `cbt::RowSet` class that groups row keys and row ranges
 * into buckets, as determined by the row key samples.
 *
 * This is sort of the point of the exercise. The implementation is here for
 * copying. A class like this may make it into the public API one day.
 *
 * Usage:
 *
 * @code
 * auto samples = table.SampleRows();
 * assert(samples.ok());
 * BucketedRowSet buckets(*std::move(samples));
 * buckets.Append("row1");
 * buckets.Append(RowRange::Open("row2", "row3"));
 * auto row_sets = std::move(buckets).RowSets();
 * for (auto& row_set : row_sets) {
 *   table.AsyncReadRows(..., row_set, ...);
 * }
 * @endcode
 */
class BucketedRowSet {
 public:
  // Initializes `buckets_`, given the row key samples.
  //
  // For example, if given sample row keys: {"1", "2"}, it will make buckets:
  // ("", "1"], ("1", "2"], ("2", ""]
  //
  // If given sample row keys: {"", "1", "2", ""}, it will still make buckets:
  // ("", "1"], ("1", "2"], ("2", ""]
  //
  // Note that `cbt::RowRange` handles the empty string differently when it is
  // the opening key vs. the closing key.
  explicit BucketedRowSet(std::vector<cbt::RowKeySample> samples) {
    buckets_.reserve(samples.size() + 1);

    cbt::RowKeyType last;
    for (auto& sample : samples) {
      buckets_.emplace_back(std::move(last), sample.row_key);
      last = std::move(sample.row_key);
    }
    if (!last.empty()) buckets_.emplace_back(std::move(last), "");
    if (buckets_.empty()) buckets_.emplace_back("", "");
  }

  void Append(cbt::RowRange range) {
    // The API is too restrictive for us to recover the starting row key. Or
    // make claims about which side of the row range a row key is on. :shrug:
    // So we need to manipulate the proto instead.
    auto proto = std::move(range).as_proto();

    auto bucket = [this, &proto] {
      switch (proto.start_key_case()) {
        case google::bigtable::v2::RowRange::START_KEY_NOT_SET:
          return buckets_.begin();
        case google::bigtable::v2::RowRange::kStartKeyClosed:
          return FindBucketClosed(proto.start_key_closed());
        case google::bigtable::v2::RowRange::kStartKeyOpen:
          return FindBucketOpen(proto.start_key_open());
      }
    }();

    // Restore the `cbt::RowRange` from the proto.
    range = cbt::RowRange(proto);

    // Perform a linear search until we hit all of the buckets that this range
    // falls into.
    while (bucket != buckets_.end()) {
      auto p = bucket->range.Intersect(range);
      // No intersection. We can stop.
      if (!p.first) break;
      bucket->empty = false;
      bucket->row_set.Append(std::move(p.second));
      ++bucket;
    }
  }

  void Append(cbt::RowKeyType row_key) {
    auto bucket = FindBucketClosed(row_key);
    bucket->empty = false;
    bucket->row_set.Append(std::move(row_key));
  }

  // Consume this object and return a vector of non-empty `cbt::`RowSets`.
  std::vector<cbt::RowSet> RowSets() && {
    std::vector<cbt::RowSet> v;
    v.reserve(buckets_.size());
    for (auto& bucket : buckets_) {
      if (bucket.empty) continue;
      v.emplace_back(std::move(bucket.row_set));
    }
    return v;
  }

 private:
  struct Bucket {
    Bucket(cbt::RowKeyType lower_bound, cbt::RowKeyType upper_bound)
        : upper_bound(upper_bound),
          range(cbt::RowRange::LeftOpen(std::move(lower_bound),
                                        std::move(upper_bound))) {}

    // We use this key to perform a binary search to determine which bucket a
    // given row key or row range belongs to. Although this information is
    // contained in `range`, the public API is too opaque for us to avoid an
    // extra copy.
    cbt::RowKeyType upper_bound;
    cbt::RowRange const range;
    // The default constructed `cbt::RowSet()` means read all rows, so we should
    // manually track whether it is empty or not.
    bool empty = true;
    cbt::RowSet row_set;
  };

  std::vector<Bucket>::iterator FindBucketClosed(
      cbt::RowKeyType const& row_key) {
    return std::lower_bound(
        buckets_.begin(), buckets_.end(), row_key,
        [](Bucket const& bucket, cbt::RowKeyType const& row_key) {
          if (bucket.upper_bound.empty()) return false;
          return bucket.upper_bound < row_key;
        });
  }

  std::vector<Bucket>::iterator FindBucketOpen(cbt::RowKeyType const& row_key) {
    return std::upper_bound(
        buckets_.begin(), buckets_.end(), row_key,
        [](cbt::RowKeyType const& row_key, Bucket const& bucket) {
          if (bucket.upper_bound.empty()) return true;
          return row_key < bucket.upper_bound;
        });
  }

  std::vector<Bucket> buckets_;
};

/**
 * Abstract interface for sourcing row range data to test with.
 *
 * The point of this abstraction is to handle row ranges that may come from a
 * file (if you are testing an existing table with real data), or from default
 * test data that is generated by this application when `create_table == true`.
 *
 * There is probably no need for this class in a real application.
 */
class RowRangeSource {
 public:
  // If the optional is not engaged, there are no more row ranges.
  virtual absl::optional<cbt::RowRange> Next() = 0;
};

/**
 * Reads RowRanges from a file.
 *
 * The input for the file is expected to be in the format:
 *
 * <start_row_key_1>, <end_row_key_1>
 * <start_row_key_2>, <end_row_key_2>
 * <start_row_key_3>, <end_row_key_3>
 * ...
 * <start_row_key_N>, <end_row_key_N>
 */
std::unique_ptr<RowRangeSource> MakeRowRangeSourceFromFile(
    std::string const& filepath);

/**
 * Generates RowRanges programmatically for testing.
 *
 * The row ranges are determined by the configuration. The row keys are of the
 * form "rowXXXX". This matches the test data generated by
 * `ConditionallyCreateTable()`.
 */
std::unique_ptr<RowRangeSource> MakeDefaultRowRangeSource(
    Configuration const& config);

// Read row ranges from a file, if a file is provided. Otherwise use the default
// test row ranges.
std::unique_ptr<RowRangeSource> MakeRowRangeSource(
    Configuration const& config) {
  if (!config.row_ranges_filepath.empty()) {
    return MakeRowRangeSourceFromFile(config.row_ranges_filepath);
  }
  return MakeDefaultRowRangeSource(config);
}

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

  if (config.debug_log) {
    std::cout << "CONFIGURATION:\n"
              << "Total Rows: " << config.total_rows << "\n"
              << "Channel Count: " << config.grpc_num_channels << "\n"
              << "Thread Count: " << config.grpc_num_threads << "\n"
              << "\nSETUP:\n";
  }

  ConditionallyCreateTable(config);

  // Make a Table client to connect to the Bigtable Data API.
  auto options =
      gc::Options{}
          .set<gc::EndpointOption>(config.data_endpoint)
          .set<gc::GrpcNumChannelsOption>(config.grpc_num_channels)
          .set<gc::GrpcBackgroundThreadPoolSizeOption>(config.grpc_num_threads);
  cbt::Table table(cbt::MakeDataConnection(options),
                   cbt::TableResource(config.project_id, config.instance_id,
                                      config.table_id));

  if (config.debug_log) std::cout << "Start sampling row keys.\n";
  auto samples_sor = table.SampleRows();
  if (!samples_sor) throw std::move(samples_sor).status();
  auto samples = *std::move(samples_sor);
  if (config.debug_log) std::cout << "Finished sampling row keys.\n";

  if (config.debug_log) std::cout << "Start reading ranges.\n";
  BucketedRowSet buckets(samples);
  auto source = MakeRowRangeSource(config);
  for (;;) {
    auto range = source->Next();
    if (!range) break;
    buckets.Append(*std::move(range));
  }
  auto row_sets = std::move(buckets).RowSets();
  if (config.debug_log) std::cout << "Finished reading ranges.\n";

  if (!config.debug_log) {
    // Print labels in csv format.
    std::cout << "ElapsedTime(ms),TotalRows,ChannelCount,ThreadCount,Buckets\n";
  }
  for (int i = 1; i <= config.num_iterations; ++i) {
    if (config.debug_log) std::cout << "\nIteration: " << i << "\n";
    TimedReadRows(table, row_sets, config);
  }

  ConditionallyDeleteTable(config);

  return 0;
} catch (gc::Status const& status) {
  std::cerr << "google::cloud::Status thrown: " << status << "\n";
  return 1;
}

// Anything in this anonymous namespace is an implementation detail that is a
// distraction from the point of the sample. (The point of the sample is to
// group row ranges into buckets that match the table's splits and parallelize
// the reads across those buckets).
namespace {

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

class FromFileRowRangeSource : public RowRangeSource {
 public:
  explicit FromFileRowRangeSource(std::string const& filepath) {
    auto is = std::ifstream(filepath);
    is.exceptions(std::ios::badbit);
    auto contents = std::string(std::istreambuf_iterator<char>(is.rdbuf()), {});
    lines_ = absl::StrSplit(contents, '\n', absl::SkipEmpty());
    it_ = lines_.begin();
  }

  absl::optional<cbt::RowRange> Next() override {
    while (it_ != lines_.end()) {
      auto line = *it_++;
      std::vector<std::string> keys = absl::StrSplit(line, ", ");
      if (keys.size() != 2) {
        std::cerr << "bad line: " << line << "\n";
        continue;
      }
      return cbt::RowRange::Closed(std::move(keys[0]), std::move(keys[1]));
    }
    return absl::nullopt;
  }

 private:
  std::vector<std::string> lines_;
  std::vector<std::string>::iterator it_;
};

std::unique_ptr<RowRangeSource> MakeRowRangeSourceFromFile(
    std::string const& filepath) {
  return std::make_unique<FromFileRowRangeSource>(filepath);
}

class DefaultRowRangeSource : public RowRangeSource {
 public:
  explicit DefaultRowRangeSource(Configuration const& config)
      : total_rows_(config.total_rows),
        disjoint_row_ranges_(config.disjoint_row_ranges),
        rows_per_range_(config.rows_per_range),
        row_key_width_(RowKeyWidth(total_rows_)) {
    // Add a random offset to the disjoint row ranges so we are not reading the
    // same subset every time.
    std::int64_t max_offset =
        total_rows_ / disjoint_row_ranges_ - rows_per_range_;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::int64_t> dist(
        0, std::max<std::int64_t>(0, max_offset - 1));
    offset_ = dist(gen);
  }

  absl::optional<cbt::RowRange> Next() override {
    if (current_row_range_ >= disjoint_row_ranges_) return absl::nullopt;
    auto start_index = current_row_range_ * total_rows_ / disjoint_row_ranges_;
    auto start_row_key = MakeRowString(row_key_width_, start_index);
    auto end_index = start_index + rows_per_range_;
    auto end_row_key = MakeRowString(row_key_width_, end_index);
    ++current_row_range_;
    return cbt::RowRange::LeftOpen(std::move(start_row_key),
                                   std::move(end_row_key));
  }

 private:
  std::int64_t total_rows_;
  std::int64_t disjoint_row_ranges_;
  std::int64_t rows_per_range_;
  int row_key_width_;
  std::int64_t offset_;

  std::int64_t current_row_range_ = 0;
};

std::unique_ptr<RowRangeSource> MakeDefaultRowRangeSource(
    Configuration const& config) {
  return std::make_unique<DefaultRowRangeSource>(config);
}

void WriteTableData(Configuration const& config) {
  if (!config.create_table) return;

  auto const row_key_width = RowKeyWidth(config.total_rows);
  auto const value = std::string(config.bytes_per_row, '0');

  auto options =
      gc::Options{}
          .set<gc::EndpointOption>(config.data_endpoint)
          .set<gc::GrpcNumChannelsOption>(config.grpc_num_channels)
          .set<gc::GrpcBackgroundThreadPoolSizeOption>(config.grpc_num_threads);
  cbt::Table table(cbt::MakeDataConnection(options),
                   cbt::TableResource(config.project_id, config.instance_id,
                                      config.table_id));

  cbt::MutationBatcher batcher(table);
  gc::CompletionQueue cq;
  std::thread cq_runner([&cq]() { cq.Run(); });

  if (config.debug_log) std::cout << "Start writing data.\n";
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
  if (config.debug_log) std::cout << "Finished writing data.\n";

  cq.Shutdown();
  cq_runner.join();
}

void ConditionallyCreateTable(Configuration const& config) {
  if (!config.create_table) return;

  auto const row_key_width = RowKeyWidth(config.total_rows);
  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);
  auto options = gc::Options{}.set<gc::EndpointOption>(config.admin_endpoint);
  auto client = cbta::BigtableTableAdminClient(
      cbta::MakeBigtableTableAdminConnection(std::move(options)));

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

  if (config.debug_log) std::cout << "Start creating table.\n";
  auto status = client.CreateTable(r);
  if (!status.ok()) throw std::move(status).status();
  if (config.debug_log) std::cout << "Finished creating table.\n";

  WriteTableData(config);
}

void ConditionallyDeleteTable(Configuration const& config) {
  if (!config.delete_table) return;

  auto const tr = cbt::TableResource(config.project_id, config.instance_id,
                                     config.table_id);
  if (config.debug_log) std::cout << "Start deleting table.\n";
  auto options = gc::Options{}.set<gc::EndpointOption>(config.admin_endpoint);
  auto client = cbta::BigtableTableAdminClient(
      cbta::MakeBigtableTableAdminConnection(std::move(options)));
  auto status = client.DeleteTable(tr.FullName());
  if (!status.ok()) throw std::move(status);
  if (config.debug_log) std::cout << "Finished deleting table.\n";
}

}  // namespace
