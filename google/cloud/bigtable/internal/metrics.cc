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
#include "google/cloud/common_options.h"
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
    {"method", labels.method},
      {"streaming", labels.streaming},
      {"client_name", labels.client_name},
      //{"client_uid", labels.client_uid},
      {"project_id", labels.project_id},
      {"instance", labels.instance},
      {"table", labels.table},
      {"app_profile", labels.app_profile},
      {"cluster", labels.cluster},
      {"zone", labels.zone},
      {"status", labels.status},
  };
}

MetricCollector::MetricCollector() {
  // TODO : what is the project?
  auto project = Project("dbolduc-test");

  // Hardcode the monitored resource override, for now.
  // TODO : support dynamic MRs.
  auto monitored_resource = google::api::MonitoredResource{};
  monitored_resource.set_type("bigtable_client_raw");
  auto& labels = *monitored_resource.mutable_labels();
  labels["project_id"] = "dbolduc-test";
  labels["instance"] = "test-instance";
  labels["cluster"] = "test-cluster";
  labels["table"] = "endurance";
  labels["zone"] = "us-east4-a";

  auto o =
      Options{}
          .set<LoggingComponentsOption>({"rpc"})
          .set<otel::ServiceTimeSeriesOption>(true)
          .set<otel::MonitoredResourceOption>(std::move(monitored_resource))
          .set<otel::MetricNameFormatterOption>([](auto name) {
            return "bigtable.googleapis.com/internal/client/" + name;
          });
  auto exporter = otel::MakeMonitoringExporter(std::move(project), std::move(o));
  auto options =
      opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions{};
  // Empirically, it seems like 30s is the minimum.
  options.export_interval_millis = std::chrono::seconds(30);
  options.export_timeout_millis = std::chrono::seconds(1);

  auto reader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(
      std::move(exporter), options);

  auto context = opentelemetry::sdk::metrics::MeterContextFactory::Create(
      std::make_unique<opentelemetry::sdk::metrics::ViewRegistry>(),
      // NOTE : this skips OTel's built in resource detection which is more
      // confusing than helpful. (The default is {{"service_name", "unknown_service" }}). And after #14930, this gets copied into our resource labels. oh god why.
      opentelemetry::sdk::resource::Resource::GetEmpty());
  context->AddMetricReader(std::move(reader));
  provider_ = opentelemetry::sdk::metrics::MeterProviderFactory::Create(
      std::move(context));

  auto meter = provider_->GetMeter("bigtable", "");
  attempt_latencies_= meter->CreateDoubleHistogram("attempt_latencies");
}

void MetricCollector::RecordAttemptLatency(double value, LabelMap const& labels) const {
    auto context = opentelemetry::context::RuntimeContext::GetCurrent();
    attempt_latencies_->Record(value, labels, context);
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google
