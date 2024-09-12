cmake_minimum_required(VERSION 3.28)

project(Google_tests)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# adding the Google_Tests_run target
add_executable(Google_Tests_run acceptance_test.cpp)

# linking Google_Tests_run with OrderBook which will be tested
target_link_libraries(Google_Tests_run OrderBook_lib)

target_link_libraries(Google_Tests_run gtest gtest_main)