// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "google/cloud/bigtable/table.h"
#include "google/cloud/opentelemetry/monitoring_exporter.h"
#include "google/cloud/opentelemetry/resource_detector.h"
#include "google/cloud/opentelemetry_options.h"
#include "google/cloud/version.h"
#include <grpcpp/ext/otel_plugin.h>
#include <opentelemetry/exporters/ostream/metric_exporter_factory.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/metrics/aggregation/default_aggregation.h>
#include <opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/metrics/push_metric_exporter.h>
#include <opentelemetry/sdk/metrics/view/instrument_selector_factory.h>
#include <opentelemetry/sdk/metrics/view/meter_selector_factory.h>
#include <opentelemetry/sdk/metrics/view/view_factory.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

namespace gc = ::google::cloud;
namespace bigtable = ::google::cloud::bigtable;
namespace metrics_sdk     = opentelemetry::sdk::metrics;

int main(int argc, char **argv)
{
  // NOTE : Uh oh. It seems that if I create a gRPC channel, something gets
  // initiated, which then disallows me from registering the builder.
  //
  // It is very unfortunate that I cannot create a monitoring client before
  // registering the builder when I need a monitoring client to register the
  // builder.
  //
  // I did think of a way around this. The GCM exporter can hold options until
  // it needs to make an export call. Only when it needs to export for the first
  // time would it turn those options into a monitoring client.
  //
  // The error:
  // E0322 20:45:19.065115861 2305117 core_configuration.cc:53] ASSERTION
  // FAILED: config_.load(std::memory_order_relaxed) == nullptr &&
  // "CoreConfiguration was already instantiated before builder " "registration
  // was completed"

  enum class Exporter { kGCM, kGCMDelayed, kOStream };
  auto exporter = [](Exporter e) {
    if (e == Exporter::kGCM) {
      auto conn = gc::monitoring_v3::MakeMetricServiceConnection();
      return gc::otel_internal::MakeMonitoringExporter(
          gc::Project("dbolduc-test"), conn,
          gc::otel_internal::MetricType::kGoogle);
    }
    if (e == Exporter::kGCMDelayed) {
      return gc::otel_internal::MakeMonitoringExporter(
          gc::Project("dbolduc-test"), nullptr,
          gc::otel_internal::MetricType::kCustom);
    }
    return opentelemetry::exporter::metrics::OStreamMetricExporterFactory::
        Create();
  }(Exporter::kGCMDelayed);
  auto detector = gc::otel::MakeResourceDetector();
  auto resource = detector->Detect();

  std::string version{"1.2.0"};
  std::string schema{"https://opentelemetry.io/schemas/1.2.0"};

  // Initialize and set the global MeterProvider
  metrics_sdk::PeriodicExportingMetricReaderOptions options;
  // NOTE : Seems like GCM enforces no more than 1 update per 5s
  options.export_interval_millis = std::chrono::milliseconds(5000);
  options.export_timeout_millis  = std::chrono::milliseconds(500);

  auto reader =
      metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), options);

  auto u_provider = metrics_sdk::MeterProviderFactory::Create(
      std::make_unique<opentelemetry::sdk::metrics::ViewRegistry>(), resource);
  auto *p         = static_cast<metrics_sdk::MeterProvider *>(u_provider.get());
  p->AddMetricReader(std::move(reader));
  // Register a global gRPC OpenTelemetry plugin configured with a GCM exporter.
  std::cout << "DEBUG : Registering plugin." << std::endl;
  auto status = grpc::OpenTelemetryPluginBuilder()
                    .SetMeterProvider(std::move(u_provider))
                    .BuildAndRegisterGlobal();
  if (!status.ok()) {
    std::cerr << "Failed to register gRPC OpenTelemetry Plugin: "
              << status.ToString() << std::endl;
    return static_cast<int>(status.code());
  }
  std::cout << "DEBUG : Successfully registered plugin?" << std::endl;

  // Use some Cloud C++ library
  auto data_conn = bigtable::MakeDataConnection(
      gc::Options{}.set<gc::OpenTelemetryTracingOption>(true));
  std::cout << "DEBUG : Successfully created Bigtable client?" << std::endl;
  auto table = bigtable::Table(
      data_conn, bigtable::TableResource("dbolduc-test", "instance", "table-gcm"));
  int i = 0;
  while (++i) {
    auto mut = bigtable::SetCell("fam", "cq", std::chrono::milliseconds(0),
                                 std::to_string(i));
    auto s = table.Apply(
        bigtable::SingleRowMutation("row" + std::to_string(i % 100), mut));
    if (!s.ok()) std::cout << s << std::endl;
  }

  return 0;
}
