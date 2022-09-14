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
#include "google/cloud/trace/exporter/gcp_exporter.h"
#include "google/cloud/trace/trace_connection.h"
#include "google/cloud/common_options.h"
#include "google/cloud/project.h"
#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>
#include <iostream>
#include <stdexcept>

namespace {

// Copy the simple example provided in the opentelemetry-cpp repo:
// https://github.com/open-telemetry/opentelemetry-cpp/blob/b8504d978d2cff1a5255ca8f55ab76f7ce6a49f7/examples/simple/main.cc
void initTracer(
    std::unique_ptr<opentelemetry::sdk::trace::SpanExporter> exporter) {
  auto processor = opentelemetry::sdk::trace::BatchSpanProcessorFactory::Create(
      std::move(exporter),
      opentelemetry::sdk::trace::BatchSpanProcessorOptions{});
  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(
          std::move(processor));

  // Set the global trace provider
  trace_api::Provider::SetTracerProvider(provider);
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " project-id\n";
    return 1;
  }

  namespace gc = ::google::cloud;

  bool export_to_cloud_trace = true;
  if (export_to_cloud_trace) {
    // Initialize the tracer with our Cloud Trace Exporter.
    auto gcp_exporter =
        absl::make_unique<google::cloud::trace_exporter::GcpExporter>(
            gc::trace::MakeTraceServiceConnection(), gc::Project(argv[1]));
    initTracer(std::move(gcp_exporter));
  } else {
    // Initialize the tracer with an exporter that prints to the command line.
    auto ostream_exporter =
        opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    initTracer(std::move(ostream_exporter));
  }

  // (Optional): Start an application level span to demonstrate how customers
  // might instrument their application.
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  auto tracer = provider->GetTracer("application tracer");
  auto span =
      opentelemetry::trace::Scope(tracer->StartSpan("application span"));

  // Create a client with OpenTelemetry tracing enabled.
  auto options =
      gc::Options{}.set<gc::experimental::OpenTelemetryTracingOption>(true);
  auto client = gc::talent::CompanyServiceClient(
      gc::talent::MakeCompanyServiceConnection(options));

  // Call the instrumented API.
  for (int i = 0; i < 2; i++) {
    (void)client.GetCompany("not-a-company");
  }
  return 0;
}
