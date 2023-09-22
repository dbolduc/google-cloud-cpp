// Copyright 2023 Google LLC
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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_DARREN_INTERNAL_RESOURCE_DETECTOR_IMPL_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_DARREN_INTERNAL_RESOURCE_DETECTOR_IMPL_H

#include "google/cloud/opentelemetry/darren/resource_detector_options.h"
#include <opentelemetry/ext/http/client/http_client_factory.h>
#include <opentelemetry/sdk/resource/resource_detector.h>

#include <deque>
#include <nlohmann/json.hpp>
#include <thread>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace detector
{
namespace gcp
{
/**
 * The internal namespace is for implementation details only. The symbols within
 * are not part of the public API. They are subject to change, including
 * deletion, without notice.
 */
namespace internal {

// Interface to simplify testing. The default will sleep.
class Retry
{
public:
  // Returns `true` if we should keep retrying, `false` if we should stop retrying.
  virtual bool OnRetry() = 0;
};

/**
 * Creates a default retry policy
 *
 * The policy is to sleep for 1s, then 2s, then 4s, then give up.
 */
std::shared_ptr<Retry> MakeDefaultRetry();

// In tests, we mock the client and the retry policy.
std::unique_ptr<sdk::resource::ResourceDetector> MakeGcpDetector(
    std::shared_ptr<ext::http::client::HttpClientSync> client,
    std::shared_ptr<Retry> retry,
    GcpDetectorOptions options = {});

}  // namespace internal
}  // namespace gcp
}  // namespace detector
OPENTELEMETRY_END_NAMESPACE

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_DARREN_INTERNAL_RESOURCE_DETECTOR_IMPL_H
