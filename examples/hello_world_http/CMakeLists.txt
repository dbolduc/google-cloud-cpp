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

cmake_minimum_required(VERSION 3.18)
project(hello-world-http LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(functions_framework_cpp REQUIRED)

add_library(functions_framework_cpp_function hello_world_http.cc)
target_link_libraries(functions_framework_cpp_function
                      functions-framework-cpp::framework)
