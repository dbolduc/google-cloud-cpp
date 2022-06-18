# syntax=docker/dockerfile:1.3-labs
# test using
#   docker buildx build --progress plain -t repro - <Dockerfile
#   PROJECT_ID=... # 
#   INSTANCE_ID=... #
#   TABLE_ID=... #
#   docker run --rm -it --volume $HOME:/h --env HOME=/h --user $UID -t repro /w/cmake-out/repro $PROJECT_ID $INSTANCE_ID $TABLE_ID

FROM debian:bullseye
ARG NCPU=48

RUN apt-get update && \
    apt-get --no-install-recommends install -y apt-transport-https apt-utils \
        automake build-essential ca-certificates ccache cmake curl git \
        gcc g++ libcurl4-openssl-dev libre2-dev \
        libssl-dev m4 make ninja-build pkg-config tar wget zlib1g-dev

WORKDIR /var/tmp/build/abseil-cpp
RUN curl -sSL https://github.com/abseil/abseil-cpp/archive/20211102.0.tar.gz | \
    tar -xzf - --strip-components=1 && \
    sed -i 's/^#define ABSL_OPTION_USE_\(.*\) 2/#define ABSL_OPTION_USE_\1 0/' "absl/base/options.h" && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=yes \
      -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig

WORKDIR /var/tmp/build/crc32c
RUN curl -sSL https://github.com/google/crc32c/archive/1.1.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCRC32C_BUILD_TESTS=OFF \
        -DCRC32C_BUILD_BENCHMARKS=OFF \
        -DCRC32C_USE_GLOG=OFF \
        -S . -B cmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig

RUN apt-get update && \
    apt-get --no-install-recommends install -y nlohmann-json3-dev

WORKDIR /var/tmp/build/protobuf
RUN curl -sSL https://github.com/protocolbuffers/protobuf/archive/v3.20.1.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Hcmake -Bcmake-out && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig

RUN apt-get update && \
    apt-get --no-install-recommends install -y libtool
WORKDIR /var/tmp/build/c-ares
RUN curl -sSL https://github.com/c-ares/c-ares/archive/cares-1_18_1.tar.gz | \
    tar -xzf - --strip-components=1 && \
    ./buildconf && ./configure && make -j ${NCPU:-4} && \
    make install && \
    ldconfig

WORKDIR /var/tmp/build/grpc
RUN curl -sSL https://github.com/grpc/grpc/archive/v1.46.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DgRPC_ABSL_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_RE2_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_ZLIB_PROVIDER=package \
        -S . -B cmake-out && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig

WORKDIR /var/tmp/build/google-cloud-cpp
RUN curl -sSL https://github.com/googleapis/google-cloud-cpp/archive/v1.40.2.tar.gz | \
    tar -xzf - --strip-components=1 && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=yes \
      -DBUILD_TESTING=OFF \
      -DGOOGLE_CLOUD_CPP_ENABLE="bigtable" \
      -S . -B cmake-out && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig

# Some of the above libraries may have installed in /usr/local, so make sure
# those library directories will be found.
RUN ldconfig /usr/local/lib*

WORKDIR /w
RUN cat >repro.cc <<_EOF_
#include "google/cloud/bigtable/table.h"
#include "google/cloud/bigtable/mutation_batcher.h"
#include "google/cloud/bigtable/admin/bigtable_table_admin_client.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace {
namespace gc = ::google::cloud;
namespace cbt = ::google::cloud::bigtable;
using ms = std::chrono::milliseconds;

bool keep_running = true;

// Polls future every 10th of a second. If the future is not ready after the
// allotted time, maybe I have reproduced the bug.
template <typename T>
void CheckFuture(gc::future<T>& f, int seconds) {
  for (int i = 0; i < seconds * 10; i++) {
    auto s = f.wait_for(ms(100));
    if (s == std::future_status::ready) break;
  }

  if (f.is_ready()) {
    f.get();
  } else {
    f.cancel();
    std::cout << "Future timed out. Maybe we have found a bug?" << std::endl;
    keep_running = false;
  }
}

class bt_batcher {
 public:
  bt_batcher(cbt::Table t, std::shared_ptr<gc::CompletionQueue> const& cq)
      : cq_(cq),
        cq_runner_([cq] { cq->Run(); }),
        batcher_(std::make_shared<cbt::MutationBatcher>(std::move(t))) {}

  void batch(cbt::SingleRowMutation mutation) {
    mutations_.fetch_add(1);

    // Variables captured by this lambda will always exists, as the
    // destructor waits for all requests completion
    auto completion_callback = [=](gc::future<gc::Status> status_future) {
      auto status = status_future.get();
      if (!status.ok()) {
        failures_.fetch_add(1);
      }
    };

    auto admission_completion = batcher_->AsyncApply(*cq_, std::move(mutation));
    auto& admission_future = admission_completion.first;
    auto& completion_future = admission_completion.second;
    completion_future.then(completion_callback);
    admission_future.get();
  }

  ~bt_batcher() {
    auto f = batcher_->AsyncWaitForNoPendingRequests();
    // Will get / cancel the future if it is not ready after 300s.
    CheckFuture(f, 300);

    // Clean up
    cq_->Shutdown();
    cq_runner_.join();

    // Debug printing
    std::cout << "mutations: " << mutations_.load()
              << "\nfailures: " << failures_.load() << std::endl;
  }

  std::atomic<int> mutations_{0};
  std::atomic<int> failures_{0};
  std::shared_ptr<gc::CompletionQueue> cq_;
  std::thread cq_runner_;
  std::shared_ptr<cbt::MutationBatcher> batcher_;
};

cbt::SingleRowMutation GenerateMutation(int i) {
  std::ostringstream os;
  os << "row" << std::setw(10) << std::setfill('0') << i;
  return cbt::SingleRowMutation(std::move(os).str(),
                                cbt::SetCell("fam", "col", ms(1), "1"));
}

}  // namespace

int main(int argc, char* argv[]) try {
  if (argc != 4) {
    std::string const cmd = argv[0];
    auto last_slash = std::string(cmd).find_last_of('/');
    std::cerr << "Usage: " << cmd.substr(last_slash + 1)
              << " <project_id> <instance_id> <table_id>\n";
    return 1;
  }

  std::string const project_id = argv[1];
  std::string const instance_id = argv[2];
  std::string const table_id = argv[3];

  namespace gc = ::google::cloud;
  namespace cbt = ::google::cloud::bigtable;
  namespace cbta = ::google::cloud::bigtable_admin;

  auto admin =
      cbta::BigtableTableAdminClient(cbta::MakeBigtableTableAdminConnection());
  auto const table_name = cbt::TableName(project_id, instance_id, table_id);

  auto constexpr kNumTrials = 10000;
  auto constexpr kMutations = 1000000;
  int trial = 0;
  while (trial < kNumTrials && keep_running)
  {
    std::cout << "Trial #" << ++trial << std::endl;

    // Every once in a while, drop all rows from the table in case that matters.
    if (trial % 10 == 1) {
      google::bigtable::admin::v2::DropRowRangeRequest req;
      req.set_name(table_name);
      req.set_delete_all_data_from_table(true);
      admin.DropRowRange(req);
    }

    {
      cbt::Table table(cbt::MakeDataClient(project_id, instance_id), table_id);
      auto cq = std::make_shared<gc::CompletionQueue>();
      bt_batcher batcher(table, cq);

      for (int i = 0; i < kMutations; i++) {
        batcher.batch(GenerateMutation(i));
      }
    }
    // batcher is destroyed here
  }
  return keep_running ? 0 : 1;
} catch (std::exception const& ex) {
  std::cerr << "Standard C++ exception raised: " << ex.what() << "\n";
  return 1;
}
_EOF_

RUN cat >CMakeLists.txt <<'_EOF_'
cmake_minimum_required(VERSION 3.16)
project(repro-batcher CXX)

find_package(google_cloud_cpp_bigtable REQUIRED)

add_executable(repro repro.cc)
target_link_libraries(repro google-cloud-cpp::bigtable)
_EOF_

RUN openssl version

RUN cmake \
      -DCMAKE_CXX_STANDARD=14 \
      -DCMAKE_BUILD_TYPE=Release \
      -G Ninja -S . -B cmake-out && \
    cmake --build cmake-out
