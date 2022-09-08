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
#include "absl/memory/memory.h"
#include "absl/types/optional.h"
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
#include <memory>
#include <string>
#include <vector>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {
namespace {

#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
using ::testing::ElementsAre;

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
  EXPECT_THAT(names, ElementsAre("test-span"));
}

#else

TEST(NoOpenTelemetryTest, Placeholder) {}

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY

}  // namespace
}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
