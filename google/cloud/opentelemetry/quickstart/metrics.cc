// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "google/cloud/version.h"
#include "google/cloud/opentelemetry/monitoring_exporter.h"
#include "google/cloud/opentelemetry/resource_detector.h"
// Examples headers don't get "installed"?
#include "foo_library.h"
//#include <opentelemetry/examples/common/metrics_foo_library/foo_library.h>
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
#include <iostream>
#include <memory>
#include <thread>

namespace gc = ::google::cloud;
namespace metrics_sdk     = opentelemetry::sdk::metrics;
namespace common          = opentelemetry::common;
namespace exportermetrics = opentelemetry::exporter::metrics;
namespace metrics_api     = opentelemetry::metrics;

namespace
{

void InitMetrics(const std::string &name)
{
  //auto exporter = exportermetrics::OStreamMetricExporterFactory::Create();
  auto conn = gc::monitoring_v3::MakeMetricServiceConnection();
  auto exporter = gc::otel_internal::MakeMonitoringExporter(
      gc::Project("dbolduc-test"), conn);
  auto detector = gc::otel::MakeResourceDetector();
  auto resource = detector->Detect();

  std::string version{"1.2.0"};
  std::string schema{"https://opentelemetry.io/schemas/1.2.0"};

  // Initialize and set the global MeterProvider
  metrics_sdk::PeriodicExportingMetricReaderOptions options;
  // NOTE : Seems like GCM enforces no more than 1 update per 5s
  //options.export_interval_millis = std::chrono::milliseconds(1000);
  options.export_interval_millis = std::chrono::milliseconds(5000);
  options.export_timeout_millis  = std::chrono::milliseconds(500);

  auto reader =
      metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), options);

  auto u_provider = metrics_sdk::MeterProviderFactory::Create(
      std::make_unique<opentelemetry::sdk::metrics::ViewRegistry>(), resource);
  auto *p         = static_cast<metrics_sdk::MeterProvider *>(u_provider.get());

  p->AddMetricReader(std::move(reader));

  // counter view
  std::string counter_name = name + "_counter";
  std::string unit         = "counter-unit";

  auto instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
      metrics_sdk::InstrumentType::kCounter, counter_name, unit);

  auto meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

  auto sum_view = metrics_sdk::ViewFactory::Create(name, "description", unit,
                                                   metrics_sdk::AggregationType::kSum);

  p->AddView(std::move(instrument_selector), std::move(meter_selector), std::move(sum_view));

  // observable counter view
  std::string observable_counter_name = name + "_observable_counter";

  auto observable_instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
      metrics_sdk::InstrumentType::kObservableCounter, observable_counter_name, unit);

  auto observable_meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

  auto observable_sum_view = metrics_sdk::ViewFactory::Create(name, "test_description", unit,
                                                              metrics_sdk::AggregationType::kSum);

  p->AddView(std::move(observable_instrument_selector), std::move(observable_meter_selector),
             std::move(observable_sum_view));

  // histogram view
  std::string histogram_name = name + "_histogram";
  unit                       = "histogram-unit";

  auto histogram_instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
      metrics_sdk::InstrumentType::kHistogram, histogram_name, unit);

  auto histogram_meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

  auto histogram_aggregation_config = std::unique_ptr<metrics_sdk::HistogramAggregationConfig>(
      new metrics_sdk::HistogramAggregationConfig);

  histogram_aggregation_config->boundaries_ = std::vector<double>{
      0.0, 50.0, 100.0, 250.0, 500.0, 750.0, 1000.0, 2500.0, 5000.0, 10000.0, 20000.0};

  std::shared_ptr<metrics_sdk::AggregationConfig> aggregation_config(
      std::move(histogram_aggregation_config));

  auto histogram_view = metrics_sdk::ViewFactory::Create(
      name, "description", unit, metrics_sdk::AggregationType::kHistogram, aggregation_config);

  p->AddView(std::move(histogram_instrument_selector), std::move(histogram_meter_selector),
             std::move(histogram_view));

  std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(std::move(u_provider));
  metrics_api::Provider::SetMeterProvider(provider);
}

void CleanupMetrics()
{
  std::shared_ptr<metrics_api::MeterProvider> none;
  metrics_api::Provider::SetMeterProvider(none);
}
}  // namespace

int main(int argc, char **argv)
{
  std::string example_type;
  if (argc >= 2)
  {
    example_type = argv[1];
  }

  std::string name{"ostream_metric_example"};
  InitMetrics(name);

  if (example_type == "counter")
  {
    foo_library::counter_example(name);
  }
  else if (example_type == "observable_counter")
  {
    foo_library::observable_counter_example(name);
  }
  else if (example_type == "histogram")
  {
    foo_library::histogram_example(name);
  }
  else if (example_type == "darren") {
    auto provider = metrics_api::Provider::GetMeterProvider();
    opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter =
        provider->GetMeter(name, "1.2.0");
    auto double_counter = meter->CreateDoubleCounter("darren");

    for (uint32_t i = 1; i < 10000; ++i) {
      double val = (rand() % 50 * i) + 1.1;
      double_counter->Add(val);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  } else {
    std::thread counter_example{&foo_library::counter_example, name};
    std::thread observable_counter_example{
        &foo_library::observable_counter_example, name};
    std::thread histogram_example{&foo_library::histogram_example, name};

    counter_example.join();
    observable_counter_example.join();
    histogram_example.join();
  }

  CleanupMetrics();
  return 0;
}
