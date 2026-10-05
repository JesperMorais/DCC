# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_node)

# Every .c file in src/ is part of the app. CONFIGURE_DEPENDS re-globs on each
# build, so a new file is picked up without a pristine build.
file(GLOB app_sources CONFIGURE_DEPENDS src/*.c)
target_sources(app PRIVATE ${app_sources})
