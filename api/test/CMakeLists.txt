# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(core)
add_subdirectory(context)
add_subdirectory(plugin)
add_subdirectory(nostd)
add_subdirectory(trace)
add_subdirectory(metrics)
if(WITH_LOGS_PREVIEW)
  add_subdirectory(logs)
endif()
add_subdirectory(common)
add_subdirectory(baggage)
add_subdirectory(singleton)
