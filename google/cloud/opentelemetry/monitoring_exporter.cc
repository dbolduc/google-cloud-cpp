// Copyright 2024 Google LLC
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

#include "google/cloud/opentelemetry/monitoring_exporter.h"
#include "google/cloud/monitoring/v3/metric_client.h"
#include "google/cloud/opentelemetry/internal/monitored_resource.h"
#include "google/cloud/internal/time_utils.h"
#include "google/cloud/project.h"
// TODO : ?
#include <google/api/monitored_resource.pb.h>
#include <opentelemetry/sdk/metrics/export/metric_producer.h>
#include <memory>

namespace google {
namespace cloud {
namespace otel_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace {

// TODO : also shows up in monitored_resource.cc
std::string AsString(
    opentelemetry::sdk::common::OwnedAttributeValue attribute) {
  if (absl::holds_alternative<std::string>(attribute)) {
    return absl::get<std::string>(attribute);
  }
  // TODO(#11775) - convert other attribute types to strings
  // Note that our resource detector only uses string attributes.
  return {};
}

google::protobuf::Timestamp ToProtoTimestamp(
    opentelemetry::common::SystemTimestamp ts) {
  return internal::ToProtoTimestamp(
      absl::FromUnixNanos(ts.time_since_epoch().count()));
}

google::api::MetricDescriptor::ValueType ToValueType(
    opentelemetry::sdk::metrics::InstrumentValueType value_type) {
  switch (value_type) {
    case opentelemetry::sdk::metrics::InstrumentValueType::kInt:
    case opentelemetry::sdk::metrics::InstrumentValueType::kLong:
      return google::api::MetricDescriptor::INT64;
    case opentelemetry::sdk::metrics::InstrumentValueType::kFloat:
    case opentelemetry::sdk::metrics::InstrumentValueType::kDouble:
      return google::api::MetricDescriptor::DOUBLE;
  }
}

google::monitoring::v3::TypedValue ToValue(
    opentelemetry::sdk::metrics::ValueType value) {
  google::monitoring::v3::TypedValue proto;
  if (absl::holds_alternative<double>(value)) {
    proto.set_double_value(absl::get<double>(value));
  } else {
    proto.set_int64_value(absl::get<std::int64_t>(value));
  }
  return proto;
}

// Stolen from OTLP exporter:
// https://github.com/open-telemetry/opentelemetry-cpp/blob/c82306f431592ceb2f7d57d2bc41e37d864c794f/exporters/otlp/src/otlp_metric_utils.cc#L27
opentelemetry::sdk::metrics::AggregationType GetAggregationType(
    opentelemetry::sdk::metrics::MetricData const& metric_data) noexcept {
  if (metric_data.point_data_attr_.empty()) {
    return opentelemetry::sdk::metrics::AggregationType::kDrop;
  }
  auto point_data_with_attributes = metric_data.point_data_attr_[0];
  if (opentelemetry::nostd::holds_alternative<
          opentelemetry::sdk::metrics::SumPointData>(
          point_data_with_attributes.point_data)) {
    return opentelemetry::sdk::metrics::AggregationType::kSum;
  }
  if (opentelemetry::nostd::holds_alternative<
          opentelemetry::sdk::metrics::HistogramPointData>(
          point_data_with_attributes.point_data)) {
    return opentelemetry::sdk::metrics::AggregationType::kHistogram;
  }
  if (opentelemetry::nostd::holds_alternative<
          opentelemetry::sdk::metrics::LastValuePointData>(
          point_data_with_attributes.point_data)) {
    return opentelemetry::sdk::metrics::AggregationType::kLastValue;
  }
  return opentelemetry::sdk::metrics::AggregationType::kDrop;
}

google::api::Distribution
HistogramToDistribution(opentelemetry::sdk::metrics::HistogramPointData h) {

}

// https://github.com/GoogleCloudPlatform/opentelemetry-operations-go/blob/babed4870546b78cee69606726961cfd20cbea42/exporter/metric/metric.go#L514
google::monitoring::v3::CreateTimeSeriesRequest ToRequest(
    opentelemetry::sdk::metrics::ResourceMetrics const& data) {
  google::monitoring::v3::CreateTimeSeriesRequest request;

  google::api::MonitoredResource mr_proto;
  auto const* resource = data.resource_;
  if (resource) {
    auto mr = ToMonitoredResource(resource->GetAttributes());
    mr_proto.set_type(std::move(mr.type));
    for (auto& label : mr.labels) {
      mr_proto.mutable_labels()->emplace(std::move(label.first),
                                         std::move(label.second));
    }
    // TODO : Ok, what do I do with mr_proto?
  }

  for (auto const& scope_metric : data.scope_metric_data_) {
    auto const* scope = scope_metric.scope_;
    for (auto const& metric_data : scope_metric.metric_data_) {
      google::monitoring::v3::TimeSeries ts;
      auto const value_type =
          ToValueType(metric_data.instrument_descriptor.value_type_);

      auto start_ts = ToProtoTimestamp(metric_data.start_ts);
      auto end_ts = ToProtoTimestamp(metric_data.end_ts);

      auto aggregation_type = GetAggregationType(metric_data);

      // point attributes become labels?? This feels wrong.
      for (auto const& pda : metric_data.point_data_attr_) {
        for (auto const& kv_attr : pda.attributes) {
          auto& labels = *ts.mutable_metric()->mutable_labels();
          labels[kv_attr.first] = AsString(kv_attr.second);
        }
      }

      // metric_data.aggregation_temporality

      switch (aggregation_type) {
        case opentelemetry::sdk::metrics::AggregationType::kHistogram:
          // ConvertHistogramMetric
          *ts.mutable_resource() = mr_proto;
          ts.set_unit(metric_data.instrument_descriptor.unit_);
          ts.set_metric_kind(google::api::MetricDescriptor::CUMULATIVE);
          ts.set_value_type(google::api::MetricDescriptor::DISTRIBUTION);
          for (auto const& pda : metric_data.point_data_attr_) {
            auto& p = *ts.add_points();
            // TODO : need to make sure end_ts - start_ts >= 1ms, so we override the end value.
            // https://github.com/GoogleCloudPlatform/opentelemetry-operations-go/blob/babed4870546b78cee69606726961cfd20cbea42/exporter/metric/metric.go#L604-L609
            if (metric_data.end_ts.time_since_epoch() -
                    metric_data.start_ts.time_since_epoch() <
                std::chrono::milliseconds(1)) {
              end_ts = internal::ToProtoTimestamp(
                  absl::FromUnixNanos((metric_data.start_ts.time_since_epoch() +
                                       std::chrono::milliseconds(1))
                                          .count()));
            }
            *p.mutable_interval()->mutable_start_time() = start_ts;
            *p.mutable_interval()->mutable_end_time() = end_ts;
          }
          break;
        case opentelemetry::sdk::metrics::AggregationType::kLastValue: {
          // ConvertGaugeMetric
          *ts.mutable_resource() = mr_proto;
          ts.set_unit(metric_data.instrument_descriptor.unit_);
          ts.set_metric_kind(google::api::MetricDescriptor::GAUGE);
          ts.set_value_type(value_type);
          for (auto const& pda : metric_data.point_data_attr_) {
            auto& p = *ts.add_points();
            // Start timestamp left empty for gauges.
            *p.mutable_interval()->mutable_end_time() = end_ts;
            // Note to self: we know it has LastValuePointData from the switch
            // on aggregation_type
            auto gauge_data = opentelemetry::nostd::get<
                opentelemetry::sdk::metrics::LastValuePointData>(
                pda.point_data);
            *p.mutable_value() = ToValue(gauge_data.value_);
          }
          break;
        case opentelemetry::sdk::metrics::AggregationType::kSum:
          // ConvertSumMetric
          break;
        case opentelemetry::sdk::metrics::AggregationType::kDrop:
        case opentelemetry::sdk::metrics::AggregationType::kDefault:
          break;
      }

      // TODO : I am really confused by which thing I am supposed to switch on.
      // e.g. if the InstrumentType == kHistogram, do I know that the data type is HistogramPointData?
      switch (metric_data.instrument_descriptor.type_) {
        case opentelemetry::sdk::metrics::InstrumentType::kCounter:
        case opentelemetry::sdk::metrics::InstrumentType::kHistogram:
        case opentelemetry::sdk::metrics::InstrumentType::kUpDownCounter:
        case opentelemetry::sdk::metrics::InstrumentType::
            kObservableCounter:
        case opentelemetry::sdk::metrics::InstrumentType::
            kObservableUpDownCounter:
        case opentelemetry::sdk::metrics::InstrumentType::kObservableGauge:
          break;
        }
        default:
          continue;
      }

      metric_data.instrument_descriptor.description_;
      metric_data.instrument_descriptor.name_;
      // metric_data.instrument_descriptor.type_;  // counter, histogram, etc.
      // metric_data.instrument_descriptor.unit_;
      // metric_data.instrument_descriptor.value_type_;  // int, long, float, dub

      for (auto const& pda : metric_data.point_data_attr_) {
        pda.attributes;  // OrderedAttributeMap
        pda.point_data;  // variant<SumPointData, HistogramPointData, LastValuePointData, DropPointData>;
                         // https://github.com/open-telemetry/opentelemetry-cpp/blob/main/sdk/include/opentelemetry/sdk/metrics/data/point_data.h
      }
    }
  }
  return {};
}

class MonitoringExporter final
    : public opentelemetry::sdk::metrics::PushMetricExporter {
 public:
  explicit MonitoringExporter(
      std::shared_ptr<monitoring_v3::MetricServiceConnection> conn)
      : client_(std::move(conn)) {}

  opentelemetry::sdk::metrics::AggregationTemporality GetAggregationTemporality(
      opentelemetry::sdk::metrics::InstrumentType) const noexcept override {
    return opentelemetry::sdk::metrics::AggregationTemporality::kCumulative;
  }

  opentelemetry::sdk::common::ExportResult Export(
      opentelemetry::sdk::metrics::ResourceMetrics const& data) noexcept
      override {
    auto request = ToRequest(data);
    auto status = client_.CreateServiceTimeSeries(request);
    // TODO(dbolduc): Log on error
    return status.ok() ? opentelemetry::sdk::common::ExportResult::kSuccess
                       : opentelemetry::sdk::common::ExportResult::kFailure;
  }

  bool ForceFlush(std::chrono::microseconds) noexcept override { return false; }

  bool Shutdown(std::chrono::microseconds) noexcept override { return true; }

 private:
  monitoring_v3::MetricServiceClient client_;
};
}  // namespace

std::unique_ptr<opentelemetry::sdk::metrics::PushMetricExporter>
MakeMonitoringExporter(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    Project project,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    std::shared_ptr<monitoring_v3::MetricServiceConnection> conn) {
  (void)project;
  (void)conn;
  return nullptr;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace otel_internal
}  // namespace cloud
}  // namespace google
