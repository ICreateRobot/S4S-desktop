cmake_minimum_required(VERSION 3.10)
project(main)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory("../../lib/udcheck" ${CMAKE_CURRENT_BINARY_DIR}/udcheck_bin)

add_executable(main main.c)
target_link_libraries(main udcheck)
