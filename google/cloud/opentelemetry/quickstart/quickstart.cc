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

//! [all]
#include "google/cloud/bigtable/table.h"
#include "google/cloud/opentelemetry/configure_basic_tracing.h"
#include "google/cloud/internal/opentelemetry.h"
#include "google/cloud/opentelemetry_options.h"
#include <iostream>
#include <thread>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " <project-id> <instance-id> <table-id>\n";
    return 1;
  }

  // Create aliases to make the code easier to read.
  namespace gc = ::google::cloud;
  namespace cbt = ::google::cloud::bigtable;

  // This is the project ID used for Cloud Trace, and the one used in the
  // Bigtable table.
  std::string const project_id = argv[1];
  std::string const instance_id = argv[2];
  std::string const table_id = argv[3];
  cbt::TableResource tr(project_id, instance_id, table_id);

  // Instantiate a basic tracing configuration which exports traces to Cloud
  // Trace. By default, spans are sent in batches and always sampled.
  auto project = gc::Project(project_id);
  auto configuration = gc::otel::ConfigureBasicTracing(project);

  // Create a client with OpenTelemetry tracing enabled.
  auto options =
      gc::Options{}
          .set<gc::OpenTelemetryTracingOption>(true)
          .set<cbt::MaxConnectionRefreshOption>(std::chrono::milliseconds(0));
  auto conn = cbt::MakeDataConnection(options);

  auto table = cbt::Table(std::move(conn), std::move(tr));

  gc::Status status;
  {
    auto darren_span = gc::internal::MakeSpan("darren");
    auto darren_scope = opentelemetry::trace::Scope(darren_span);

    cbt::SingleRowMutation m(
        "r1", cbt::SetCell("cf1", "cq", std::chrono::milliseconds(0), "value"));
    auto f = table.AsyncApply(m)
                 .then([](auto f) {
                   auto s = f.get();
                   auto span = gc::internal::MakeSpan("then1");
                   auto scope = opentelemetry::trace::Scope(span);
                   auto child = gc::internal::MakeSpan("then1 child");
                   std::this_thread::sleep_for(std::chrono::milliseconds(100));
                   child->SetStatus(opentelemetry::trace::StatusCode::kError);
                   child->End();
                   span->End();
                   return s;
                 })
                 .then([](auto f) {
                   auto f2 = f.get();
                   auto span = gc::internal::MakeSpan("then2");
                   std::this_thread::sleep_for(std::chrono::milliseconds(200));
                   span->End();
                   return f2;
                 });

    status = f.get();
  }

  if (!status.ok()) {
    std::cout << "Call failed with: " << status << std::endl;
    return 1;
  }

  std::cout << "Mutations applied." << std::endl;
  return 0;
}
//! [all]
