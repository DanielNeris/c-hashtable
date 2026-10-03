cmake_minimum_required(VERSION 3.21)
project(betterhash C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CFLAGS=-Wall -g  ->  the Debug build type adds -g
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
add_compile_options(-Wall)

# libhashtable.a
add_library(hashtable_static STATIC hashtable.c)
set_target_properties(hashtable_static PROPERTIES OUTPUT_NAME hashtable)
target_include_directories(hashtable_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# libhashtable.so (libhashtable.dylib on macOS)
add_library(hashtable_shared SHARED hashtable.c)
set_target_properties(hashtable_shared PROPERTIES OUTPUT_NAME hashtable)
target_include_directories(hashtable_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# test, linked against the static library
add_executable(test test.c hashfunctions.c)
target_link_libraries(test PRIVATE hashtable_static)
