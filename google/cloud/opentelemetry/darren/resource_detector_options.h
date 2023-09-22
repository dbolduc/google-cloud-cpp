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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_DARREN_RESOURCE_DETECTOR_OPTIONS_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_DARREN_RESOURCE_DETECTOR_OPTIONS_H

#include <opentelemetry/version.h>
#include <string>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace detector
{
namespace gcp
{

/**
 * Configuration options for the GCP resource detector.
 */
struct GcpDetectorOptions
{
  /**
   * The endpoint of the Google Compute Engine (GCE) [Metadata Server]
   *
   * [metadata server]: https://cloud.google.com/compute/docs/metadata/overview
   */
  std::string endpoint = "http://metadata.google.internal";
};

}  // namespace gcp
}  // namespace detector
OPENTELEMETRY_END_NAMESPACE

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_OPENTELEMETRY_DARREN_RESOURCE_DETECTOR_OPTIONS_H
