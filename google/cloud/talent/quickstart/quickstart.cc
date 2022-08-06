// Copyright 2022 Google LLC
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

#include "google/cloud/talent/company_client.h"
#include "google/cloud/project.h"
#include "gcp_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include <iostream>
#include <stdexcept>

namespace {

// Example Trace: https://screenshot.googleplex.com/37aR8KudkDDWtnH
// Where does the extra latency come from? I think it has to be exporting the
// spans when the close.
void initTracer() {
  auto gcp_exporter = std::unique_ptr<opentelemetry::sdk::trace::SpanExporter>(
      new opentelemetry::exporter::gcp::GcpExporter());
  auto processor = std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>(
      new opentelemetry::sdk::trace::SimpleSpanProcessor(std::move(gcp_exporter)));
  auto provider = std::shared_ptr<opentelemetry::trace::TracerProvider>(
      new opentelemetry::sdk::trace::TracerProvider(std::move(processor)));

  // Set the global trace provider
  trace_api::Provider::SetTracerProvider(provider);
}

}

int main(int argc, char* argv[]) try {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " project-id\n";
    return 1;
  }

  // Initialize OT business
  initTracer();
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  auto tracer = provider->GetTracer("quickstart tracer");

  // Scoped span.
  auto run_quickstart_span =
      opentelemetry::trace::Scope(tracer->StartSpan("run_quickstart"));
  auto create_client_span = tracer->StartSpan("create_client");
  namespace talent = ::google::cloud::talent;
  auto client =
      talent::CompanyServiceClient(talent::MakeCompanyServiceConnection());
  create_client_span->End();

  // Scoped span. bc am lazy and don't want to think about the `catch`
  auto make_calls_span =
      opentelemetry::trace::Scope(tracer->StartSpan("qs list companies loop"));
  auto const project = google::cloud::Project(argv[1]);
  for (auto i = 0; i != 2; ++i) {
    for (auto c : client.ListCompanies(project.FullName())) {
      if (!c) throw std::runtime_error(c.status().message());
      std::cout << c->DebugString() << "\n";
    }
  }

  return 0;
} catch (std::exception const& ex) {
  std::cerr << "Standard exception raised: " << ex.what() << "\n";
  return 1;
}
