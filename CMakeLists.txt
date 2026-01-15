# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lorawan_etsist)

target_sources(app PRIVATE
    src/main.c
    src/drivers/humidity_sensor.c
    src/drivers/leds.c
    src/drivers/gps_sensor.c
    src/drivers/gps_thread.c
    src/drivers/light_sensor.c
    src/drivers/soil_sensor.c
    src/drivers/rgb_sensor.c
)

target_include_directories(app PRIVATE)

