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
  std::string method;
  std::string streaming;
  std::string client_name;
  std::string client_uid;
  std::string table_name;
  std::string app_profile;
  std::string cluster;
  std::string zone;
  // TODO : what is the format of status? "UNAVAILABLE"? Or an int? who knows.
  std::string status;
};

using LabelMap = std::map<std::string, std::string>;
LabelMap IntoMap(MetricLabels labels);

class Metrics {
 public:
  virtual void RecordAttemptLatency(double value,
                                    MetricLabels const& labels) const {}
  virtual void RecordOperationLatency(double value,
                                      MetricLabels const& labels) const {}
};

std::shared_ptr<Metrics> MakeMetrics();

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_BIGTABLE_INTERNAL_METRICS_H
