// Copyright 2022 Google LLC
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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TRACE_EXPORTER_GCP_EXPORTER_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TRACE_EXPORTER_GCP_EXPORTER_H

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

#include "google/cloud/trace/exporter/recordable.h"
#include "google/cloud/trace/trace_client.h"
#include "google/cloud/project.h"
#include <opentelemetry/sdk/trace/exporter.h>
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace trace_exporter {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

/**
 * This class handles all the functionality of exporting traces to Google Cloud
 */
// TODO(dbolduc): I hate the name of this class.
//                I think I prefer CloudTraceExporter.
class GcpExporter final : public opentelemetry::sdk::trace::SpanExporter {
 public:
  /**
   * Class Constructor which invokes all the register/initialization functions
   */
  explicit GcpExporter(
      std::shared_ptr<trace::TraceServiceConnection> connection,
      Project project)
      : client_(std::move(connection)), project_(std::move(project)) {}

  /**
   * Creates a Recordable(Span) object
   */
  std::unique_ptr<opentelemetry::sdk::trace::Recordable>
  MakeRecordable() noexcept override;

  /**
   * Exports all gathered spans to the cloud
   *
   * @param spans - List of spans to export to google cloud
   * @return Success or failure based on returned gRPC status
   */
  opentelemetry::sdk::common::ExportResult Export(
      opentelemetry::nostd::span<
          std::unique_ptr<opentelemetry::sdk::trace::Recordable>> const&
          spans) noexcept override;

  /**/
  bool Shutdown(std::chrono::microseconds) noexcept override { return true; }

 private:
  google::cloud::trace::TraceServiceClient client_;
  google::cloud::Project project_;
};

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace trace_exporter
}  // namespace cloud
}  // namespace google

#else

#warning "The library has not been built with OpenTelemetry enabled."

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_TRACE_EXPORTER_GCP_EXPORTER_H
