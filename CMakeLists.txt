cmake_minimum_required(VERSION 3.15)
set(CMAKE_VERBOSE_MAKEFILE ON)
get_filename_component(APP_PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${APP_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

