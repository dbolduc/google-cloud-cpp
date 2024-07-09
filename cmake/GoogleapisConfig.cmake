# ~~~
# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

# Give application developers a hook to configure the version and hash
# downloaded from GitHub.
set(GOOGLE_CLOUD_CPP_GOOGLEAPIS_COMMIT_SHA
    ""
    CACHE STRING "Deprecated. Use GOOGLE_CLOUD_CPP_OVERRIDE_GOOGLEAPIS_URL")
mark_as_advanced(GOOGLE_CLOUD_CPP_GOOGLEAPIS_COMMIT_SHA)
set(GOOGLE_CLOUD_CPP_GOOGLEAPIS_SHA256
    ""
    CACHE STRING
          "Deprecated. Use GOOGLE_CLOUD_CPP_OVERRIDE_GOOGLEAPIS_URL_HASH")
mark_as_advanced(GOOGLE_CLOUD_CPP_GOOGLEAPIS_SHA256)

set(_GOOGLE_CLOUD_CPP_GOOGLEAPIS_COMMIT_SHA
    "f681f79a93814d8b974da9dd8cdc62228d0f4758")
set(_GOOGLE_CLOUD_CPP_GOOGLEAPIS_SHA256
    "8eb7534cb188241b45bbcf72b1c425c7e62b2f95d7bfe0b723b77be3235d526b")

set(DOXYGEN_ALIASES
    "googleapis_link{2}=\"[\\1](https://github.com/googleapis/googleapis/blob/${_GOOGLE_CLOUD_CPP_GOOGLEAPIS_COMMIT_SHA}/\\2)\""
    "googleapis_reference_link{1}=\"https://github.com/googleapis/googleapis/blob/${_GOOGLE_CLOUD_CPP_GOOGLEAPIS_COMMIT_SHA}/\\1\""
)
