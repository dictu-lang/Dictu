cmake_minimum_required(VERSION 3.16.3)
PROJECT("ffi-test")
set(CMAKE_C_STANDARD 11)
add_library(ffi-test SHARED src/lib.c)

target_include_directories(ffi-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../src/include")