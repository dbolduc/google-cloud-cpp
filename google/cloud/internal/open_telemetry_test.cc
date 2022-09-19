// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/internal/open_telemetry.h"
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <opentelemetry/exporters/memory/in_memory_span_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/span_data.h>
#include <opentelemetry/sdk/trace/tracer.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <gmock/gmock.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {
namespace {

using ms = std::chrono::milliseconds;
using ::testing::MockFunction;

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
using ::testing::AllOf;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Pair;
using ::testing::Pointee;
using ::testing::Property;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;
using ::testing::VariantWith;

class OpenTelemetryTest : public ::testing::Test {
 protected:
  OpenTelemetryTest() {
    auto exporter = absl::make_unique<
        opentelemetry::exporter::memory::InMemorySpanExporter>();
    span_data_ = exporter->GetData();

    auto processor =
        absl::make_unique<opentelemetry::sdk::trace::SimpleSpanProcessor>(
            std::move(exporter));
    std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
        opentelemetry::sdk::trace::TracerProviderFactory::Create(
            std::move(processor));
    opentelemetry::trace::Provider::SetTracerProvider(provider);
  }

  std::shared_ptr<opentelemetry::exporter::memory::InMemorySpanData> span_data_;
};

TEST_F(OpenTelemetryTest, MakeSpan) {
  auto s = MakeSpan("test-span");
  s->End();

  // Verify that a span was made.
  auto spans = span_data_->GetSpans();
  std::vector<opentelemetry::nostd::string_view> names;
  names.reserve(spans.size());
  for (auto const& span : spans) names.push_back(span->GetName());
  EXPECT_THAT(names, UnorderedElementsAre("test-span"));
  EXPECT_THAT(
      spans,
      Each(Pointee(Property(&opentelemetry::sdk::trace::SpanData::GetSpanKind,
                            opentelemetry::trace::SpanKind::kClient))));
}

TEST_F(OpenTelemetryTest, CaptureStatusDetailsEndSpan) {
  auto closed = MakeSpan("closed-span");
  CaptureStatusDetails(*closed, Status(), true);
  auto spans = span_data_->GetSpans();
  EXPECT_THAT(spans, SizeIs(1));

  auto open = MakeSpan("open-span");
  CaptureStatusDetails(*open, Status(), false);
  spans = span_data_->GetSpans();
  EXPECT_THAT(spans, IsEmpty());
  open->End();
}

TEST_F(OpenTelemetryTest, CaptureStatusDetailsSuccess) {
  auto success = MakeSpan("success");
  CaptureStatusDetails(*success, Status(), true);

  auto spans = span_data_->GetSpans();
  EXPECT_THAT(spans,
              UnorderedElementsAre(Pointee(AllOf(
                  Property(&opentelemetry::sdk::trace::SpanData::GetStatus,
                           opentelemetry::trace::StatusCode::kOk),
                  Property(&opentelemetry::sdk::trace::SpanData::GetAttributes,
                           UnorderedElementsAre(
                               Pair("gcloud.status_code",
                                    VariantWith<std::string>("OK"))))))));
}

TEST_F(OpenTelemetryTest, CaptureStatusDetailsFail) {
  auto fail = MakeSpan("fail");
  CaptureStatusDetails(*fail, Status(StatusCode::kAborted, "not good"), true);

  auto spans = span_data_->GetSpans();
  EXPECT_THAT(spans,
              UnorderedElementsAre(Pointee(AllOf(
                  Property(&opentelemetry::sdk::trace::SpanData::GetStatus,
                           opentelemetry::trace::StatusCode::kError),
                  Property(&opentelemetry::sdk::trace::SpanData::GetAttributes,
                           UnorderedElementsAre(
                               Pair("gcloud.status_code",
                                    VariantWith<std::string>("ABORTED")),
                               Pair("gcloud.status_message",
                                    VariantWith<std::string>("not good"))))))));
}

TEST_F(OpenTelemetryTest, CaptureStatusDetailsTruncates) {
  std::string truncated(128, 'A');
  auto span = MakeSpan("span");
  CaptureStatusDetails(*span, Status(StatusCode::kAborted, truncated + "tail"),
                       true);

  auto spans = span_data_->GetSpans();
  EXPECT_THAT(spans,
              UnorderedElementsAre(Pointee(AllOf(
                  Property(&opentelemetry::sdk::trace::SpanData::GetStatus,
                           opentelemetry::trace::StatusCode::kError),
                  Property(&opentelemetry::sdk::trace::SpanData::GetAttributes,
                           UnorderedElementsAre(
                               Pair("gcloud.status_code",
                                    VariantWith<std::string>("ABORTED")),
                               Pair("gcloud.status_message",
                                    VariantWith<std::string>(truncated))))))));
}

TEST_F(OpenTelemetryTest, CaptureReturn) {
  auto v1 = StatusOr<int>(5);
  auto v2 = StatusOr<int>(Status(StatusCode::kAborted, "fail"));

  auto s1 = MakeSpan("s1");
  auto r1 = CaptureReturn(*s1, v1, true);
  EXPECT_EQ(r1, v1);

  auto s2 = MakeSpan("s2");
  auto r2 = CaptureReturn(*s2, v2, true);
  EXPECT_EQ(r2, v2);

  // Let's confirm that the status was set for these spans.
  auto spans = span_data_->GetSpans();
  EXPECT_THAT(spans, SizeIs(2));
  EXPECT_THAT(
      spans,
      Each(Pointee(Property(&opentelemetry::sdk::trace::SpanData::GetStatus,
                            Not(opentelemetry::trace::StatusCode::kUnset)))));
}

TEST_F(OpenTelemetryTest, MakeTracingSleeper) {
  MockFunction<void(ms)> mock_sleeper;
  EXPECT_CALL(mock_sleeper, Call(ms(42)));

  auto sleeper = mock_sleeper.AsStdFunction();
  auto result = MakeTracingSleeper("test::function", sleeper);
  result(ms(42));

  // Verify that a span was made.
  auto spans = span_data_->GetSpans();
  std::vector<opentelemetry::nostd::string_view> names;
  names.reserve(spans.size());
  for (auto const& span : spans) names.push_back(span->GetName());
  EXPECT_THAT(names, UnorderedElementsAre("test::function::backoff"));
}

#else

TEST(NoOpenTelemetryTest, MakeTracingSleeper) {
  MockFunction<void(ms)> mock_sleeper;
  EXPECT_CALL(mock_sleeper, Call(ms(42)));

  auto sleeper = mock_sleeper.AsStdFunction();
  auto result = MakeTracingSleeper("test::function", sleeper);
  result(ms(42));
}

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

}  // namespace
}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
