# SPDX-License-Identifier: Apache-2.0
#
# The test image: every file in ../src except main.c, plus the ztest suites
# in src/. -DMILESTONE=m2 builds only that milestone's suite, -DMILESTONE=all
# (the default) builds them all.
cmake_minimum_required(VERSION 3.20.0)

get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

# The app's bindings (dts/bindings/) and its prj.conf, then ours on top so
# the test settings win. The devicetree comes from tests/app.overlay, not the
# app's: the tests need sensors with known values.
list(APPEND DTS_ROOT ${APP_ROOT})
set(CONF_FILE "${APP_ROOT}/prj.conf;${CMAKE_CURRENT_SOURCE_DIR}/prj.conf")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_node_tests)

set(MILESTONE all CACHE STRING "Which milestone's tests to build: m1 to m5, or all")

file(GLOB app_sources CONFIGURE_DEPENDS ${APP_ROOT}/src/*.c)
list(REMOVE_ITEM app_sources ${APP_ROOT}/src/main.c)

if(MILESTONE STREQUAL "all")
  file(GLOB test_sources src/test_*.c)
else()
  set(test_sources src/test_${MILESTONE}.c)
endif()

target_sources(app PRIVATE ${app_sources} ${test_sources})
target_include_directories(app PRIVATE ${APP_ROOT}/src)
