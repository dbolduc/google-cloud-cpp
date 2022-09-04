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
#include "google/cloud/internal/scoped_span.h"
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
#include <chrono>
#include <functional>
#include <thread>

namespace google {
namespace cloud {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

std::function<void(std::chrono::milliseconds)> MaybeMakeTracingSleeper(
    std::string const& func,  // NOLINT(misc-unused-parameters)
    std::function<void(std::chrono::milliseconds)> sleeper) {
  return [&func, sleeper](std::chrono::milliseconds p) {
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
    // print for debugging.
    std::cout << "MakeSpan(" + func + "::backoff)\n";
    auto span = MakeSpan(func + "::backoff");
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
    sleeper(p);
#ifdef GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
    // TODO(dbolduc) - I should just use a Scope
    span->End();
#endif  // GOOGLE_CLOUD_CPP_HAVE_OPEN_TELEMETRY
  };
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace cloud
}  // namespace google
