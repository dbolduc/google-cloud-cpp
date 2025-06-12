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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_METRICS_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_METRICS_H

// TODO : write the class, assuming OTel, then clean up the gates.
//#ifdef GOOGLE_CLOUD_CPP_BIGTABLE_WITH_OTEL_METRICS

#include "google/cloud/bigtable/version.h"
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <map>
#include <string>

namespace google {
namespace cloud {
namespace bigtable_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

// It will be convenient to have one struct
struct MetricLabels {
  // TODO : consider avoiding copies in the no-CSM case with references
  std::string method_name;
  std::string streaming_operation;
  std::string client_name;
  std::string project_id;
  std::string instance_id;
  std::string table_id;
  std::string app_profile_id;
  std::string cluster;
  std::string zone;
  // TODO : what is the format of status? "UNAVAILABLE"? Or an int? who knows.
  std::string status;
};

using LabelMap = std::map<std::string, std::string>;
LabelMap IntoMap(MetricLabels labels);


/**
 * Something that knows how to collect metrics
 */
class MetricCollector {
 public:
   MetricCollector();

   void RecordAttemptLatency(double value, LabelMap const& labels);

 private:
   // NOTE : probably don't need to hold the provider.
  std::shared_ptr<opentelemetry::metrics::MeterProvider> provider_;
  std::shared_ptr<opentelemetry::metrics::Histogram<double>> attempt_latencies_;
};

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_METRICS_H
