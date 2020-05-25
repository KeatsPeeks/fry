cmake_minimum_required(VERSION 3.15)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(fry)

if (BUILD_VERSION)
    add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output)

add_executable(${CMAKE_PROJECT_NAME} main.cpp main.h)

