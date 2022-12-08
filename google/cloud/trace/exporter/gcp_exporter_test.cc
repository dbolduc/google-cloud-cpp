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

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
#include "google/cloud/trace/exporter/gcp_exporter.h"
#include "google/cloud/trace/mocks/mock_trace_connection.h"
#include "google/cloud/version.h"
#include <gtest/gtest.h>
#include <opentelemetry/common/timestamp.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/provider.h>
#include <cstdlib>

namespace google {
namespace cloud {
namespace trace_exporter {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace {

namespace nostd = opentelemetry::nostd;
namespace sdk = opentelemetry::sdk;

using testing::_;
using testing::AtLeast;
using testing::Return;

TEST(GcpExporterTest, TestGeneralFunctionality) {
  auto mock = std::make_shared<trace_mocks::MockTraceServiceConnection>();
  EXPECT_CALL(*mock, BatchWriteSpans)
      .Times(AtLeast(1))
      .WillRepeatedly(Return(Status()));

  auto gcp_exporter =
      absl::make_unique<GcpExporter>(mock, Project("test-project"));
  auto processor = std::unique_ptr<sdk::trace::SpanProcessor>(
      absl::make_unique<sdk::trace::SimpleSpanProcessor>(
          std::move(gcp_exporter)));
  auto provider = std::shared_ptr<opentelemetry::trace::TracerProvider>(
      absl::make_unique<sdk::trace::TracerProvider>(std::move(processor)));

  auto tracer = provider->GetTracer("Test Export");

  auto test_span = tracer->StartSpan("Test Span");

  auto span_1 = tracer->StartSpan("Span 1");
  auto span_1_1 = tracer->StartSpan("Span 1.1");
  auto span_1_1_1 = tracer->StartSpan("Span 1.1.1");
  span_1_1_1->End();
  span_1_1->End();
  span_1->End();

  auto span_2 = tracer->StartSpan("Span 2");
  auto span_2_1 = tracer->StartSpan("Span 2.1");
  span_2_1->End();
  auto span_2_2 = tracer->StartSpan("Span 2.2");
  span_2_2->End();
  span_2->End();

  test_span->End();
}

TEST(GcpExporterTest, TestExportResults) {
  using Batch = nostd::span<std::unique_ptr<sdk::trace::Recordable>>;

  auto mock = std::make_shared<trace_mocks::MockTraceServiceConnection>();
  EXPECT_CALL(*mock, BatchWriteSpans)
      .WillOnce(Return(Status()))
      .WillOnce(Return(Status(StatusCode::kPermissionDenied, "fail")));

  auto gcp_exporter =
      absl::make_unique<GcpExporter>(mock, Project("test-project"));

  // Make sample recordables
  auto recordable_1 = gcp_exporter->MakeRecordable();
  recordable_1->SetName("Sample span 1");
  auto recordable_2 = gcp_exporter->MakeRecordable();
  recordable_2->SetName("Sample span 2");

  // Test Success
  Batch const batch_1(&recordable_1, 1);
  auto result_1 = gcp_exporter->Export(batch_1);
  EXPECT_EQ(sdk::common::ExportResult::kSuccess, result_1);

  // Test Failure
  Batch const batch_2(&recordable_2, 1);
  auto result_2 = gcp_exporter->Export(batch_2);
  EXPECT_EQ(sdk::common::ExportResult::kFailure, result_2);
}

}  // namespace
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace trace_exporter
}  // namespace cloud
}  // namespace google
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
