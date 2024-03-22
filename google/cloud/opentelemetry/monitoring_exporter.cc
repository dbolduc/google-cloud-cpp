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
#include "google/cloud/log.h"
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

void PopulateMetric(
    google::monitoring::v3::TimeSeries& ts,
    opentelemetry::sdk::metrics::MetricData const& metric_data,
    opentelemetry::sdk::metrics::PointAttributes const& attributes) {
  // TODO : The metric name is configurable in golang. And I think for
  // Bigtable/Storage cases, it gets set to a different value?
  auto const metric_type =
      "workload.googleapis.com/" + metric_data.instrument_descriptor.name_;
  ts.mutable_metric()->set_type(metric_type);

  // TODO(dbolduc): Seems like we have one TimeSeries / PointData? And each one
  // gets its own labels. We do not aggregate the labels.
  for (auto const& kv : attributes) {
    auto& labels = *ts.mutable_metric()->mutable_labels();
    labels[kv.first] = AsString(kv.second);
  }
}

google::api::Distribution HistogramToDistribution(
    opentelemetry::sdk::metrics::HistogramPointData const& h) {
  google::api::Distribution d;
  d.set_count(h.count_);

  if (h.count_ > 0) {
    //double mean = std::accumulate(h.counts_.begin(), h.counts_.end(), 0.0) /
    //              static_cast<double>(h.count_);
    double sum = absl::holds_alternative<double>(h.sum_)
                     ? absl::get<double>(h.sum_)
                     : static_cast<double>(absl::get<int64_t>(h.sum_));
    d.set_mean(sum / h.count_);
  }
  for (auto v : h.counts_) {
    // TODO(dbolduc): could use this loop for the average instead of looping
    // twice.
    d.add_bucket_counts(v);
  }
  for (auto b : h.boundaries_) {
    d.mutable_bucket_options()->mutable_explicit_buckets()->add_bounds(b);
  }
  // TODO(dbolduc) : calculate variance?
  // https://github.com/GoogleCloudPlatform/opentelemetry-operations-go/blob/babed4870546b78cee69606726961cfd20cbea42/exporter/metric/metric.go#L585
  //
  // It makes absolutely no sense for the client to do this calculation. Or to
  // calculate the mean. We are already sending the relevant information... wtf.
  return d;
}

void PopulateHistogram(
    google::monitoring::v3::CreateTimeSeriesRequest& request,
    google::api::MonitoredResource const& resource,
    opentelemetry::sdk::metrics::MetricData const& metric_data) {
  auto start_ts = ToProtoTimestamp(metric_data.start_ts);
  // We need to make sure end_ts - start_ts >= 1ms. To achieve this, we
  // override the end value.
  // https://github.com/GoogleCloudPlatform/opentelemetry-operations-go/blob/babed4870546b78cee69606726961cfd20cbea42/exporter/metric/metric.go#L604-L609
  auto end_ts_nanos = (std::max)(
      metric_data.end_ts.time_since_epoch(),
      metric_data.start_ts.time_since_epoch() + std::chrono::milliseconds(1));
  auto end_ts =
      internal::ToProtoTimestamp(absl::FromUnixNanos(end_ts_nanos.count()));

  for (auto const& pda : metric_data.point_data_attr_) {
    auto& ts = *request.add_time_series();
    ts.set_unit(metric_data.instrument_descriptor.unit_);
    ts.set_metric_kind(google::api::MetricDescriptor::DELTA);
    ts.set_value_type(google::api::MetricDescriptor::DISTRIBUTION);
    *ts.mutable_resource() = resource;
    PopulateMetric(ts, metric_data, pda.attributes);

    auto& p = *ts.add_points();
    *p.mutable_interval()->mutable_start_time() = start_ts;
    *p.mutable_interval()->mutable_end_time() = end_ts;
    // Note to self: we know it has HistogramPointData from the switch
    // on aggregation_type
    auto histogram_data = opentelemetry::nostd::get<
        opentelemetry::sdk::metrics::HistogramPointData>(pda.point_data);
    *p.mutable_value()->mutable_distribution_value() =
        HistogramToDistribution(histogram_data);
  }
}

void PopulateGauge(
    google::monitoring::v3::CreateTimeSeriesRequest& request,
    google::api::MonitoredResource const& resource,
    opentelemetry::sdk::metrics::MetricData const& metric_data) {
  //auto start_ts = ToProtoTimestamp(metric_data.start_ts);
  auto end_ts = ToProtoTimestamp(metric_data.end_ts);
  auto const value_type =
      ToValueType(metric_data.instrument_descriptor.value_type_);

  for (auto const& pda : metric_data.point_data_attr_) {
    auto& ts = *request.add_time_series();
    ts.set_unit(metric_data.instrument_descriptor.unit_);
    ts.set_metric_kind(google::api::MetricDescriptor::GAUGE);
    ts.set_value_type(value_type);
    *ts.mutable_resource() = resource;
    PopulateMetric(ts, metric_data, pda.attributes);

    auto& p = *ts.add_points();
    // Start timestamp left empty for gauges.
    *p.mutable_interval()->mutable_end_time() = end_ts;
    auto gauge_data = opentelemetry::nostd::get<
        opentelemetry::sdk::metrics::LastValuePointData>(pda.point_data);
    *p.mutable_value() = ToValue(gauge_data.value_);
  }
}

void PopulateSum(
    google::monitoring::v3::CreateTimeSeriesRequest& request,
    google::api::MonitoredResource const& resource,
    opentelemetry::sdk::metrics::MetricData const& metric_data) {
  auto const start_ts = ToProtoTimestamp(metric_data.start_ts);
  // We need to make sure end_ts - start_ts >= 1ms. To achieve this, we
  // override the end value.
  // https://github.com/GoogleCloudPlatform/opentelemetry-operations-go/blob/babed4870546b78cee69606726961cfd20cbea42/exporter/metric/metric.go#L604-L609
  auto end_ts_nanos = (std::max)(
      metric_data.end_ts.time_since_epoch(),
      metric_data.start_ts.time_since_epoch() + std::chrono::milliseconds(1));
  auto const end_ts =
      internal::ToProtoTimestamp(absl::FromUnixNanos(end_ts_nanos.count()));
  auto const value_type =
      ToValueType(metric_data.instrument_descriptor.value_type_);

  for (auto const& pda : metric_data.point_data_attr_) {
    auto& ts = *request.add_time_series();
    ts.set_unit(metric_data.instrument_descriptor.unit_);
    ts.set_metric_kind(google::api::MetricDescriptor::CUMULATIVE);
    ts.set_value_type(value_type);
    *ts.mutable_resource() = resource;
    PopulateMetric(ts, metric_data, pda.attributes);

    auto& p = *ts.add_points();
    *p.mutable_interval()->mutable_start_time() = start_ts;
    *p.mutable_interval()->mutable_end_time() = end_ts;
    auto sum_data =
        opentelemetry::nostd::get<opentelemetry::sdk::metrics::SumPointData>(
            pda.point_data);
    *p.mutable_value() = ToValue(sum_data.value_);
  }
}

// https://github.com/GoogleCloudPlatform/opentelemetry-operations-go/blob/babed4870546b78cee69606726961cfd20cbea42/exporter/metric/metric.go#L514
google::monitoring::v3::CreateTimeSeriesRequest ToRequest(
    opentelemetry::sdk::metrics::ResourceMetrics const& data) {
  google::monitoring::v3::CreateTimeSeriesRequest request;

  google::api::MonitoredResource resource;
  if (data.resource_) {
    auto mr = ToMonitoredResource(data.resource_->GetAttributes());
    resource.set_type(std::move(mr.type));
    for (auto& label : mr.labels) {
      resource.mutable_labels()->emplace(std::move(label.first),
                                         std::move(label.second));
    }
  }

  for (auto const& scope_metric : data.scope_metric_data_) {
    auto const* scope = scope_metric.scope_;
    for (auto const& metric_data : scope_metric.metric_data_) {
      auto aggregation_type = GetAggregationType(metric_data);
      switch (aggregation_type) {
        case opentelemetry::sdk::metrics::AggregationType::kHistogram:
          std::cout << "DARREN : Histogram" << std::endl;
          PopulateHistogram(request, resource, metric_data);
          break;
        case opentelemetry::sdk::metrics::AggregationType::kLastValue:
          std::cout << "DARREN : LastValue" << std::endl;
          PopulateGauge(request, resource, metric_data);
          break;
        case opentelemetry::sdk::metrics::AggregationType::kSum:
          std::cout << "DARREN : Sum" << std::endl;
          PopulateSum(request, resource, metric_data);
          break;
        case opentelemetry::sdk::metrics::AggregationType::kDrop:
          std::cout << "DARREN : Drop" << std::endl;
        case opentelemetry::sdk::metrics::AggregationType::kDefault:
          break;
      }

      // TODO : I am really confused by which thing I am supposed to switch
      // on. e.g. if the InstrumentType == kHistogram, do I know that the
      // data type is HistogramPointData?
      switch (metric_data.instrument_descriptor.type_) {
        case opentelemetry::sdk::metrics::InstrumentType::kCounter:
        case opentelemetry::sdk::metrics::InstrumentType::kHistogram:
        case opentelemetry::sdk::metrics::InstrumentType::kUpDownCounter:
        case opentelemetry::sdk::metrics::InstrumentType::kObservableCounter:
        case opentelemetry::sdk::metrics::InstrumentType::
            kObservableUpDownCounter:
        case opentelemetry::sdk::metrics::InstrumentType::kObservableGauge:
          break;
        default:
          continue;
      }

      // TODO : I have not used these fields anywhere
      //metric_data.aggregation_temporality
      //metric_data.instrument_descriptor.description_;
    }
  }
  return request;
}

class MonitoringExporter final
    : public opentelemetry::sdk::metrics::PushMetricExporter {
 public:
  explicit MonitoringExporter(
      Project project,
      std::shared_ptr<monitoring_v3::MetricServiceConnection> conn)
      : project_(std::move(project)), client_(std::move(conn)) {}

  opentelemetry::sdk::metrics::AggregationTemporality GetAggregationTemporality(
      opentelemetry::sdk::metrics::InstrumentType) const noexcept override {
    return opentelemetry::sdk::metrics::AggregationTemporality::kCumulative;
  }

  opentelemetry::sdk::common::ExportResult Export(
      opentelemetry::sdk::metrics::ResourceMetrics const& data) noexcept
      override {
    auto request = ToRequest(data);
    request.set_name(project_.FullName());
    if (request.time_series().empty()) {
      GCP_LOG(WARNING) << "Cloud Monitoring Export skipped. No TimeSeries "
                          "added to request...";
      return opentelemetry::v1::sdk::common::ExportResult::kSuccess;
    }

    // NOTE : The exporter used for client side metrics should use
    // `CreateServiceTimeSeries`. While I am developing though, testing with
    // OTel's simple metric example, I will use the standard `CreateTimeSeries`.
    //
    // auto status = client_.CreateServiceTimeSeries(request);
    auto status = client_.CreateTimeSeries(request);

    if (status.ok()) return opentelemetry::sdk::common::ExportResult::kSuccess;
    GCP_LOG(WARNING) << "Cloud Monitoring Export failed with status=" << status;
    return opentelemetry::sdk::common::ExportResult::kFailure;
  }

  bool ForceFlush(std::chrono::microseconds) noexcept override { return false; }

  bool Shutdown(std::chrono::microseconds) noexcept override { return true; }

 private:
  Project project_;
  monitoring_v3::MetricServiceClient client_;
};
}  // namespace

std::unique_ptr<opentelemetry::sdk::metrics::PushMetricExporter>
MakeMonitoringExporter(
    Project project,
    std::shared_ptr<monitoring_v3::MetricServiceConnection> conn) {
  return std::make_unique<MonitoringExporter>(std::move(project),
                                              std::move(conn));
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace otel_internal
}  // namespace cloud
}  // namespace google
