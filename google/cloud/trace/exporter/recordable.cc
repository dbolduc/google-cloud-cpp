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

#include "google/cloud/trace/exporter/recordable.h"

namespace google {
namespace cloud {
namespace trace_exporter {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

namespace common = opentelemetry::common;
namespace nostd = opentelemetry::nostd;

constexpr size_t kAttributeStringLen = 256;
constexpr size_t kDisplayNameStringLen = 128;

// Taken from the Unilib namespace
// Link:
// http://35.193.25.4/TensorFlow/models/research/syntaxnet/util/utf8/unilib_utf8_utils.h
bool IsTrailByte(char x) { return static_cast<signed char>(x) < -0x40; }

void SetTruncatableString(
    int const limit, nostd::string_view string_name,
    google::devtools::cloudtrace::v2::TruncatableString* str) {
  if (limit < 0 || string_name.size() < limit) {
    str->set_value(string_name.data());
    str->set_truncated_byte_count(0);
    return;
  }

  // If limit points to beginning of utf8 character, truncate at the limit,
  // backtrack to the beginning of utf8 character otherwise.
  int truncation_pos = limit;

  while (truncation_pos > 0 && IsTrailByte(string_name[truncation_pos])) {
    --truncation_pos;
  }

  const size_t original_size = string_name.size();
  auto truncated_str = string_name.substr(0, truncation_pos);
  str->set_value(std::string(truncated_str));
  str->set_truncated_byte_count(original_size - str->value().size());
}

void Recordable::SetIdentity(
    opentelemetry::trace::SpanContext const& span_context,
    opentelemetry::trace::SpanId parent_span_id) noexcept {
  std::array<char, 2 * opentelemetry::trace::TraceId::kSize> hex_trace_buf;
  span_context.trace_id().ToLowerBase16(hex_trace_buf);
  const std::string hex_trace(hex_trace_buf.data(), 2 * opentelemetry::trace::TraceId::kSize);

  std::array<char, 2 * opentelemetry::trace::SpanId::kSize> hex_span_buf;
  span_context.span_id().ToLowerBase16(hex_span_buf);
  const std::string hex_span(hex_span_buf.data(), 2 * opentelemetry::trace::SpanId::kSize);

  std::array<char, 2 * opentelemetry::trace::SpanId::kSize> hex_parent_span_buf;
  parent_span_id.ToLowerBase16(hex_parent_span_buf);
  const std::string hex_parent_span(hex_parent_span_buf.data(),
                                    2 * opentelemetry::trace::SpanId::kSize);

  // TODO : we will leave off the "projects/<project-id>" and let the exporter
  //        prepend it.
  span_.set_name(kTracesPathStr + hex_trace + kSpansPathStr + hex_span);
  //span_.set_name(kProjectsPathStr + std::string{"PROJECT_ID"} + kTracesPathStr +
  //               hex_trace + kSpansPathStr + hex_span);
  span_.set_span_id(hex_span);
  span_.set_parent_span_id(hex_parent_span);
}

void Recordable::SetAttribute(nostd::string_view key,
                              common::AttributeValue const& value) noexcept {
  // Get the protobuf span's map
  auto* map = span_.mutable_attributes()->mutable_attribute_map();

  if (nostd::holds_alternative<bool>(value)) {
    (*map)[key.data()].set_bool_value(nostd::get<bool>(value));
  } else if (nostd::holds_alternative<int>(value)) {
    (*map)[key.data()].set_int_value(nostd::get<int>(value));
  } else if (nostd::holds_alternative<int64_t>(value)) {
    (*map)[key.data()].set_int_value(nostd::get<int64_t>(value));
  } else if (nostd::holds_alternative<unsigned int>(value)) {
    (*map)[key.data()].set_int_value(nostd::get<unsigned int>(value));
  } else if (nostd::holds_alternative<uint64_t>(value)) {
    (*map)[key.data()].set_int_value(nostd::get<uint64_t>(value));
  } else if (nostd::holds_alternative<int64_t>(value)) {
    (*map)[key.data()].set_int_value(nostd::get<int64_t>(value));
  } else if (nostd::holds_alternative<nostd::string_view>(value)) {
    SetTruncatableString(kAttributeStringLen,
                         nostd::get<nostd::string_view>(value),
                         (*map)[key.data()].mutable_string_value());
  } else if (nostd::holds_alternative<char const*>(value)) {
    SetTruncatableString(kAttributeStringLen, nostd::get<char const*>(value),
                         (*map)[key.data()].mutable_string_value());
  }
}

void Recordable::AddEvent(
    nostd::string_view name, opentelemetry::common::SystemTimestamp timestamp,
    opentelemetry::common::KeyValueIterable const& attributes) noexcept {
  (void)name;
  (void)timestamp;
  (void)attributes;
}

void Recordable::AddLink(
    opentelemetry::trace::SpanContext const& span_context,
    opentelemetry::common::KeyValueIterable const& attributes) noexcept {
  (void)span_context;
  (void)attributes;
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

void Recordable::SetSpanKind(
    opentelemetry::trace::SpanKind span_kind) noexcept {
  span_.set_span_kind(MapSpanKind(span_kind));
}

void Recordable::SetResource(
    opentelemetry::sdk::resource::Resource const& resource) noexcept {
  // TODO : modify span_ ?
  (void)resource;
}

void Recordable::SetStartTime(
    opentelemetry::common::SystemTimestamp start_time) noexcept {
  const std::chrono::nanoseconds unix_time_nanoseconds(
      start_time.time_since_epoch().count());
  auto const seconds =
      std::chrono::duration_cast<std::chrono::seconds>(unix_time_nanoseconds);
  span_.mutable_start_time()->set_seconds(seconds.count());
  span_.mutable_start_time()->set_nanos(
      unix_time_nanoseconds.count() -
      std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}

void Recordable::SetDuration(std::chrono::nanoseconds duration) noexcept {
  const std::chrono::nanoseconds start_time_nanos(span_.start_time().nanos());
  const std::chrono::seconds start_time_seconds(span_.start_time().seconds());
  const std::chrono::nanoseconds unix_end_time(
      std::chrono::duration_cast<std::chrono::nanoseconds>(start_time_seconds)
          .count() +
      start_time_nanos.count() + duration.count());
  auto const seconds =
      std::chrono::duration_cast<std::chrono::seconds>(unix_end_time);
  span_.mutable_end_time()->set_seconds(seconds.count());
  span_.mutable_end_time()->set_nanos(
      unix_end_time.count() -
      std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}

void Recordable::SetInstrumentationLibrary(
    opentelemetry::sdk::instrumentationlibrary::InstrumentationLibrary const&
        instrumentation_library) noexcept {
  // TODO : modify span_ ?
  (void)instrumentation_library;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace trace_exporter
}  // namespace cloud
}  // namespace google
