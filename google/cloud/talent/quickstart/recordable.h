/*
 * Copyright 2021 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "google/devtools/cloudtrace/v2/tracing.grpc.pb.h"
#include "opentelemetry/sdk/trace/recordable.h"
#include "opentelemetry/version.h"
#include "opentelemetry/nostd/variant.h"


constexpr char kProjectsPathStr[] = "projects/";
constexpr char kTracesPathStr[] = "/traces/";
constexpr char kSpansPathStr[] = "/spans/";
constexpr char kGCPEnvVar[] = "GOOGLE_CLOUD_PROJECT_ID";


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{
class Recordable final : public sdk::trace::Recordable
{
public:
 google::devtools::cloudtrace::v2::Span const& span() const noexcept {
   return span_;
 }

  /**
   * Set the span context and parent span id
   * @param span_context the span context to set
   * @param parent_span_id the parent span id to set
   */
 void SetIdentity(
     opentelemetry::trace::SpanContext const& span_context,
     opentelemetry::trace::SpanId parent_span_id) noexcept override;

 /**
  * Set an attribute of a span.
  * @param name the name of the attribute
  * @param value the attribute value
  */
 void SetAttribute(
     nostd::string_view key,
     opentelemetry::common::AttributeValue const& value) noexcept override;

 /**
  * Add an event to a span.
  * @param name the name of the event
  * @param timestamp the timestamp of the event
  * @param attributes the attributes associated with the event
  */
 void AddEvent(nostd::string_view name,
               opentelemetry::common::SystemTimestamp timestamp,
               opentelemetry::common::KeyValueIterable const&
                   attributes) noexcept override;

 /**
  * Add a link to a span.
  * @param span_context the span context of the linked span
  * @param attributes the attributes associated with the link
  */
 void AddLink(opentelemetry::trace::SpanContext const& span_context,
              opentelemetry::common::KeyValueIterable const&
                  attributes) noexcept override;

 /**
  * Set the status of the span.
  * @param code the status code
  * @param description a description of the status
  */
 void SetStatus(opentelemetry::trace::StatusCode code,
                nostd::string_view description) noexcept override;

 /**
  * Set the name of the span.
  * @param name the name to set
  */
 void SetName(nostd::string_view name) noexcept override;

 /**
  * Set the spankind of the span.
  * @param span_kind the spankind to set
  */
 void SetSpanKind(opentelemetry::trace::SpanKind span_kind) noexcept override;

 /**
  * Set Resource of the span
  * @param Resource the resource to set
  */
 void SetResource(
     opentelemetry::sdk::resource::Resource const& resource) noexcept override;

 /**
  * Set the start time of the span.
  * @param start_time the start time to set
  */
 void SetStartTime(
     opentelemetry::common::SystemTimestamp start_time) noexcept override;

 /**
  * Set the duration of the span.
  * @param duration the duration to set
  */
 void SetDuration(std::chrono::nanoseconds duration) noexcept override;

 /**
  * Set the instrumentation library of the span.
  * @param instrumentation_library the instrumentation library to set
  */
 void SetInstrumentationLibrary(
     opentelemetry::sdk::instrumentationlibrary::InstrumentationLibrary const&
         instrumentation_library) noexcept override;

private:
  google::devtools::cloudtrace::v2::Span span_;
};

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE 
