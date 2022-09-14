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

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

#include "google/cloud/trace/exporter/gcp_exporter.h"
#include <grpcpp/grpcpp.h>
#include <opentelemetry/nostd/span.h>

namespace google {
namespace cloud {
namespace trace_exporter {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

namespace sdk = opentelemetry::sdk;
namespace nostd = opentelemetry::nostd;

std::unique_ptr<sdk::trace::Recordable> GcpExporter::MakeRecordable() noexcept {
  return absl::make_unique<Recordable>(project_);
}

sdk::common::ExportResult GcpExporter::Export(
    nostd::span<std::unique_ptr<sdk::trace::Recordable>> const&
        spans) noexcept {
  google::devtools::cloudtrace::v2::BatchWriteSpansRequest request;
  request.set_name(project_.FullName());
  for (auto& recordable : spans) {
    auto span = std::unique_ptr<Recordable>(
        static_cast<Recordable*>(recordable.release()));
    *request.add_spans() = std::move(span->span());
  }

  auto status = client_.BatchWriteSpans(request);
  return status.ok() ? sdk::common::ExportResult::kSuccess
                     : sdk::common::ExportResult::kFailure;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace trace_exporter
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
