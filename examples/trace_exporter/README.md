# Cloud Trace Exporter example

An example project that runs an application with OpenTelemetry tracing enabled
in the `google-cloud-cpp` library. The application exports the collected traces
to Cloud Trace.

## Run the example

```sh
bazel run //:quickstart -- $PROJECT_ID
```

Note that the `.bazelrc` file in this project is supplying the following
necessary flags:
- `--@io_opentelemetry_cpp//api:with_abseil`
- `--@com_github_googleapis_google_cloud_cpp//:experimental-open_telemetry`

## See the results

The trace created by the application can be visualized in the
[Cloud Trace console](https://console.cloud.google.com/traces).

## Example code

```cc
#include "google/cloud/talent/company_client.h"
#include "google/cloud/trace/exporter/gcp_exporter.h"
#include "google/cloud/trace/trace_connection.h"
#include "google/cloud/common_options.h"
#include "google/cloud/internal/open_telemetry.h"
#include "google/cloud/project.h"
#include <opentelemetry/context/propagation/global_propagator.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>
#include <iostream>
#include <stdexcept>

namespace {

void initTracer(
    std::unique_ptr<opentelemetry::sdk::trace::SpanExporter> exporter,
    std::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>
        propagator) {
  auto processor = opentelemetry::sdk::trace::BatchSpanProcessorFactory::Create(
      std::move(exporter),
      opentelemetry::sdk::trace::BatchSpanProcessorOptions{});
  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(
          std::move(processor));

  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);

  // Set the global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::
      SetGlobalPropagator(std::move(propagator));
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " project-id\n";
    return 1;
  }

  namespace gc = ::google::cloud;

  // Initialize the tracer using the Cloud Trace exporter and propagator.
  auto gcp_exporter =
      absl::make_unique<google::cloud::trace_exporter::GcpExporter>(
          gc::trace::MakeTraceServiceConnection(), gc::Project(argv[1]));
  auto gcp_propagator = std::make_shared<gc::internal::CloudTraceContext>();
  initTracer(std::move(gcp_exporter), std::move(gcp_propagator));

  // (Optional): Start an application level span to demonstrate how a customer
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
  (void)client.GetCompany("not-a-company-1");
  (void)client.GetCompany("not-a-company-2");

  return 0;
}
```

## Why `talent`?

We pick `talent` because it is a simple CRUD library that will not immediately
incur any billing charges.
