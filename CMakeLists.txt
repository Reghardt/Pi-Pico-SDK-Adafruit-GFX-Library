cmake_minimum_required(VERSION 3.5)

project(Adafruit_GFX)

add_library(Adafruit_GFX STATIC
    Adafruit_GrayOLED.cpp
    Adafruit_GFX.cpp
)

# Inherit the include directories from the Pico SDK
target_link_libraries(Adafruit_GFX pico_stdlib Adafruit_BusIO)
target_include_directories(Adafruit_GFX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
