// Copyright 2024 Google LLC
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

#include "google/cloud/bigtable/internal/retry_info_helper.h"
#include "google/cloud/internal/retry_loop_helpers.h"

namespace google {
namespace cloud {
namespace bigtable_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

absl::optional<std::chrono::milliseconds> BackoffOrBreak(
    bool use_server_retry_info, Status const& status, RetryPolicy& retry,
    BackoffPolicy& backoff) {
  auto delay =
      internal::BackoffOrBreak(status, "darren", retry, backoff,
                               Idempotency::kIdempotent, use_server_retry_info);
  if (!delay) return absl::nullopt;
  return *delay;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace bigtable_internal
}  // namespace cloud
}  // namespace google
