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

#include "google/cloud/trace/exporter/recordable.h"
#include "google/cloud/internal/time_utils.h"

namespace google {
namespace cloud {
namespace trace_exporter {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace {

namespace common = opentelemetry::common;
namespace nostd = opentelemetry::nostd;

std::size_t constexpr kAttributeStringLen = 256;
std::size_t constexpr kDisplayNameStringLen = 128;
std::size_t constexpr kAnnotationDescriptionLen = 256;

// Taken from the Unilib namespace
// Link:
// http://35.193.25.4/TensorFlow/models/research/syntaxnet/util/utf8/unilib_utf8_utils.h
bool IsTrailByte(char x) { return static_cast<signed char>(x) < -0x40; }

void SetTruncatableString(
    std::size_t const limit, nostd::string_view string_name,
    google::devtools::cloudtrace::v2::TruncatableString* str) {
  if (string_name.size() < limit) {
    str->set_value(string_name.data());
    str->set_truncated_byte_count(0);
    return;
  }

  // If limit points to beginning of utf8 character, truncate at the limit,
  // backtrack to the beginning of utf8 character otherwise.
  auto truncation_pos = limit;

  while (truncation_pos > 0 && IsTrailByte(string_name[truncation_pos])) {
    --truncation_pos;
  }

  auto original_size = string_name.size();
  str->set_value(string_name.data(), truncation_pos);
  str->set_truncated_byte_count(
      static_cast<std::int32_t>(original_size - truncation_pos));
}

void SetMapAttribute(
    google::protobuf::Map<
        std::string, google::devtools::cloudtrace::v2::AttributeValue>& map,
    nostd::string_view key, common::AttributeValue const& value) {
  if (nostd::holds_alternative<bool>(value)) {
    map[key.data()].set_bool_value(nostd::get<bool>(value));
  } else if (nostd::holds_alternative<int>(value)) {
    map[key.data()].set_int_value(nostd::get<int>(value));
  } else if (nostd::holds_alternative<int64_t>(
                 value)) {  // NOLINT(bugprone-branch-clone)
    map[key.data()].set_int_value(nostd::get<int64_t>(value));
  } else if (nostd::holds_alternative<unsigned int>(value)) {
    map[key.data()].set_int_value(nostd::get<unsigned int>(value));
  } else if (nostd::holds_alternative<uint64_t>(value)) {
    map[key.data()].set_int_value(nostd::get<uint64_t>(value));
  } else if (nostd::holds_alternative<int64_t>(value)) {
    map[key.data()].set_int_value(nostd::get<int64_t>(value));
  } else if (nostd::holds_alternative<nostd::string_view>(value)) {
    SetTruncatableString(kAttributeStringLen,
                         nostd::get<nostd::string_view>(value),
                         map[key.data()].mutable_string_value());
  } else if (nostd::holds_alternative<char const*>(value)) {
    SetTruncatableString(kAttributeStringLen, nostd::get<char const*>(value),
                         map[key.data()].mutable_string_value());
  }
}

::google::devtools::cloudtrace::v2::Span_SpanKind MapSpanKind(
    opentelemetry::trace::SpanKind span_kind) {
  switch (span_kind) {
    case opentelemetry::trace::SpanKind::kInternal:
      return ::google::devtools::cloudtrace::v2::Span::INTERNAL;
    case opentelemetry::trace::SpanKind::kServer:
      return ::google::devtools::cloudtrace::v2::Span::SERVER;
    case opentelemetry::trace::SpanKind::kClient:
      return ::google::devtools::cloudtrace::v2::Span::CLIENT;
    case opentelemetry::trace::SpanKind::kProducer:
      return ::google::devtools::cloudtrace::v2::Span::PRODUCER;
    case opentelemetry::trace::SpanKind::kConsumer:
      return ::google::devtools::cloudtrace::v2::Span::CONSUMER;
    default:
      return ::google::devtools::cloudtrace::v2::Span::SPAN_KIND_UNSPECIFIED;
  }
}

}  // namespace

void Recordable::SetIdentity(
    opentelemetry::trace::SpanContext const& span_context,
    opentelemetry::trace::SpanId parent_span_id) noexcept {
  std::array<char, 2 * opentelemetry::trace::TraceId::kSize> hex_trace_buf;
  span_context.trace_id().ToLowerBase16(hex_trace_buf);
  const std::string hex_trace(hex_trace_buf.data(),
                              2 * opentelemetry::trace::TraceId::kSize);

  std::array<char, 2 * opentelemetry::trace::SpanId::kSize> hex_span_buf;
  span_context.span_id().ToLowerBase16(hex_span_buf);
  const std::string hex_span(hex_span_buf.data(),
                             2 * opentelemetry::trace::SpanId::kSize);

  std::array<char, 2 * opentelemetry::trace::SpanId::kSize> hex_parent_span_buf;
  parent_span_id.ToLowerBase16(hex_parent_span_buf);
  const std::string hex_parent_span(hex_parent_span_buf.data(),
                                    2 * opentelemetry::trace::SpanId::kSize);

  span_.set_name(project_.FullName() + "/traces/" + hex_trace + "/spans/" +
                 hex_span);
  span_.set_span_id(hex_span);
  span_.set_parent_span_id(hex_parent_span);
}

void Recordable::SetAttribute(nostd::string_view key,
                              common::AttributeValue const& value) noexcept {
  // Get the protobuf span's map
  auto* map = span_.mutable_attributes()->mutable_attribute_map();
  SetMapAttribute(*map, key, value);
}

void Recordable::AddEvent(
    nostd::string_view name, opentelemetry::common::SystemTimestamp timestamp,
    opentelemetry::common::KeyValueIterable const& attributes) noexcept {
  auto& event = *span_.mutable_time_events()->add_time_event();
  *event.mutable_time() = google::cloud::internal::ToProtoTimestamp(timestamp);
  SetTruncatableString(kAnnotationDescriptionLen, name,
                       event.mutable_annotation()->mutable_description());
  auto& map = *event.mutable_annotation()
                   ->mutable_attributes()
                   ->mutable_attribute_map();
  attributes.ForEachKeyValue(
      [&map](nostd::string_view key, common::AttributeValue value) {
        SetMapAttribute(map, key, value);
        return true;
      });
}

void Recordable::AddLink(
    opentelemetry::trace::SpanContext const& span_context,
    opentelemetry::common::KeyValueIterable const& attributes) noexcept {
  char span_id[16 + 1];
  char trace_id[32 + 1];
  span_id[16] = '\0';
  trace_id[32] = '\0';
  span_context.trace_id().ToLowerBase16(
      opentelemetry::nostd::span<char,
                                 2 * opentelemetry::trace::TraceId::kSize>{
          trace_id, 32});
  span_context.span_id().ToLowerBase16(
      opentelemetry::nostd::span<char, 2 * opentelemetry::trace::SpanId::kSize>{
          span_id, 16});
  auto& link = *span_.mutable_links()->add_link();
  link.set_span_id(span_id);
  link.set_trace_id(trace_id);

  auto& map = *link.mutable_attributes()->mutable_attribute_map();
  attributes.ForEachKeyValue(
      [&map](nostd::string_view key, common::AttributeValue value) {
        SetMapAttribute(map, key, value);
        return true;
      });
}

void Recordable::SetStatus(opentelemetry::trace::StatusCode code,
                           nostd::string_view description) noexcept {
  (void)code;
  (void)description;
}

void Recordable::SetName(nostd::string_view name) noexcept {
  SetTruncatableString(kDisplayNameStringLen, name,
                       span_.mutable_display_name());
}

void Recordable::SetSpanKind(
    opentelemetry::trace::SpanKind span_kind) noexcept {
  span_.set_span_kind(MapSpanKind(span_kind));
}

void Recordable::SetResource(
    opentelemetry::sdk::resource::Resource const& resource) noexcept {
  (void)resource;
}

void Recordable::SetStartTime(
    opentelemetry::common::SystemTimestamp start_time) noexcept {
  *span_.mutable_start_time() =
      google::cloud::internal::ToProtoTimestamp(start_time);
}

void Recordable::SetDuration(std::chrono::nanoseconds duration) noexcept {
  auto const end_time =
      google::cloud::internal::ToChronoTimePoint(span_.start_time()) + duration;
  *span_.mutable_end_time() =
      google::cloud::internal::ToProtoTimestamp(end_time);
}

void Recordable::SetInstrumentationScope(
    opentelemetry::sdk::instrumentationscope::InstrumentationScope const&
        instrumentation_scope) noexcept {
  (void)instrumentation_scope;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace trace_exporter
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_HAVE_OPENTELEMETRY
