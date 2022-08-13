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

#include "google/cloud/trace/exporter/gcp_exporter.h"
#include "opentelemetry/nostd/span.h"
#include <grpcpp/grpcpp.h>

namespace google {
namespace cloud {
namespace trace_exporter {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

namespace sdk = opentelemetry::sdk;
namespace nostd = opentelemetry::nostd;

std::unique_ptr<sdk::trace::Recordable> GcpExporter::MakeRecordable() noexcept {
  return std::unique_ptr<sdk::trace::Recordable>(new Recordable);
}

sdk::common::ExportResult GcpExporter::Export(
    nostd::span<std::unique_ptr<sdk::trace::Recordable>> const&
        spans) noexcept {
  // Set up gRPC request
  google::devtools::cloudtrace::v2::BatchWriteSpansRequest request;
  request.set_name(kProjectsPathStr + project_id_);
  for (auto& recordable : spans) {
    auto span = std::unique_ptr<Recordable>(
        static_cast<Recordable*>(recordable.release()));
    auto proto = std::move(span->span());
    proto.set_name("projects/" + project_id_);
    *request.add_spans() = std::move(proto);
  }

  auto status = client_.BatchWriteSpans(request);
  if (!status.ok()) return sdk::common::ExportResult::kFailure;
  return sdk::common::ExportResult::kSuccess;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace trace_exporter
}  // namespace cloud
}  // namespace google
