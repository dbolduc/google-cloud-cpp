// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/bigtable/internal/metrics.h"
#include "google/cloud/opentelemetry/monitoring_exporter.h"
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter_context_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>

namespace google {
namespace cloud {
namespace bigtable_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

LabelMap IntoMap(MetricLabels labels) {
  return {
    {"method_name", labels.method_name},
      {"streaming_operation", labels.streaming_operation},
      {"client_name", labels.client_name},
      {"project_id", labels.project_id},
      {"instance_id", labels.instance_id},
      {"table_id", labels.table_id},
      {"app_profile_id", labels.app_profile_id},
      {"cluster", labels.cluster},
      {"zone", labels.zone},
      {"status", labels.status},
  };
}

MetricCollector::MetricCollector() {
  // TODO : what is the project?
  auto exporter = otel::MakeMonitoringExporter(Project("dbolduc-test"));
  auto options =
      opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions{};
  options.export_interval_millis = std::chrono::seconds(10);

  auto reader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(
      std::move(exporter), options);

  auto context = opentelemetry::sdk::metrics::MeterContextFactory::Create();
  context->AddMetricReader(std::move(reader));
  provider_ = opentelemetry::sdk::metrics::MeterProviderFactory::Create(
      std::move(context));

  auto meter = provider_->GetMeter("bigtable", "");
  attempt_latencies_= meter->CreateDoubleHistogram("attempt_latencies");
}

void MetricCollector::RecordAttemptLatency(double value, LabelMap const& labels) {
    auto context = opentelemetry::context::RuntimeContext::GetCurrent();
    attempt_latencies_->Record(value, labels, context);
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google
