# ~~~
# Copyright 2023 Google LLC
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

include(GoogleCloudCppLibrary)

set(GOOGLE_CLOUD_CPP_SERVICE_DIRS "v1/")

set(targets)

foreach (darren "addon_widget_set" "extension_point" "script_manifest")
    set(proto "google/apps/script/type/${darren}.proto")
    external_googleapis_short_name(short_name "${proto}")
    set(target "google_cloud_cpp_${short_name}")

    external_googleapis_add_library("${proto}")
    external_googleapis_install_pc("${target}")

    google_cloud_cpp_install_proto_library_headers("${target}")
    google_cloud_cpp_install_proto_library_protos(
        "${target}" "${EXTERNAL_GOOGLEAPIS_SOURCE}")

    list(APPEND targets "${target}")
endforeach ()

foreach (addon "calendar" "docs" "drive" "gmail" "sheets" "slides")
    set(proto "google/apps/script/type/${addon}/${addon}_addon_manifest.proto")
    external_googleapis_short_name(short_name "${proto}")
    set(target "google_cloud_cpp_${short_name}")

    # TODO : we could be more accurate here. calendar ->
    # api_field_behavior_protos docs -> api_field_behavior_protos,
    # extension_point_protos drive -> extension_point_protos gmail ->
    # addon_widget_set, extension_point_protos sheets ->
    # api_field_behavior_protos, extension_point_protos slides ->
    # api_field_behavior_protos, extension_point_protos
    external_googleapis_add_library("${proto}" api_field_behavior_protos)
    external_googleapis_install_pc("${target}")

    google_cloud_cpp_install_proto_library_headers("${target}")
    google_cloud_cpp_install_proto_library_protos(
        "${target}" "${EXTERNAL_GOOGLEAPIS_SOURCE}")

    list(APPEND targets "${target}")
endforeach ()

install(
    TARGETS ${targets}
    # TODO : do we need a new thingy?
    EXPORT googleapis-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT google_cloud_cpp_runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT google_cloud_cpp_runtime
            NAMELINK_COMPONENT google_cloud_cpp_development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT google_cloud_cpp_development)

google_cloud_cpp_add_ga_grpc_library(gsuiteaddons
                                     "Google Workspace Add-ons API")

if (BUILD_TESTING AND GOOGLE_CLOUD_CPP_ENABLE_CXX_EXCEPTIONS)
    add_executable(gsuiteaddons_quickstart "quickstart/quickstart.cc")
    target_link_libraries(gsuiteaddons_quickstart
                          PRIVATE google-cloud-cpp::gsuiteaddons)
    google_cloud_cpp_add_common_options(gsuiteaddons_quickstart)
    add_test(
        NAME gsuiteaddons_quickstart
        COMMAND cmake -P "${PROJECT_SOURCE_DIR}/cmake/quickstart-runner.cmake"
                $<TARGET_FILE:gsuiteaddons_quickstart> GOOGLE_CLOUD_PROJECT)
    set_tests_properties(gsuiteaddons_quickstart
                         PROPERTIES LABELS "integration-test;quickstart")
endif ()
