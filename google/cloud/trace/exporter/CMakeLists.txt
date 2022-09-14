# ~~~
# Copyright 2022 Google LLC
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

# TODO(dbolduc): The tests don't run in our cmake ci builds... That's not good.
message("\n\nWhat the f***\n\n")
if (NOT BUILD_TESTING)
    return()
endif ()

set(trace_exporter_unit_tests gcp_exporter_test.cc recordable_test.cc)

# Export the list of unit tests to a .bzl file so we do not need to maintain the
# list in two places.
export_list_to_bazel("trace_exporter_unit_tests.bzl"
                     "trace_exporter_unit_tests" YEAR "2022")

# Generate a target for each unit test.
foreach (fname ${trace_exporter_unit_tests})
    google_cloud_cpp_add_executable(target "trace_exporter" "${fname}")
    target_link_libraries(
        ${target}
        PRIVATE google_cloud_cpp_testing
                google_cloud_cpp_testing_grpc
                google-cloud-cpp::trace_mocks
                google-cloud-cpp::trace
                absl::str_format
                GTest::gmock_main
                GTest::gmock
                GTest::gtest)
    google_cloud_cpp_add_common_options(${target})

    # With googletest it is relatively easy to exceed the default number of
    # sections (~65,000) in a single .obj file. Add the /bigobj option to all
    # the tests, even if it is not needed.
    if (MSVC)
        target_compile_options(${target} PRIVATE "/bigobj")
    endif ()
    add_test(NAME ${target} COMMAND ${target})
endforeach ()
