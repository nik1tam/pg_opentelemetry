# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.11)

project(cwrapper)

include_directories(
  ${CMAKE_BINARY_DIR}/generated/third_party/opentelemetry-proto
  ${CMAKE_SOURCE_DIR}/exporters/otlp/include)

#add_executable(cwrapper cwrapper.cc)

add_library(cwrapper cwrapper.h cwrapper.cc)
#set_target_version(cwrapper)

target_link_libraries(cwrapper PUBLIC ${CMAKE_THREAD_LIBS_INIT}
                        opentelemetry_api opentelemetry_trace
                        opentelemetry_exporter_otlp_http)
