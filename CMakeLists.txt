cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES main.cpp)
add_executable(OrderBook_run ${SOURCE_FILES})

include_directories(OrderBook_lib)
add_subdirectory(OrderBook_lib)

target_link_libraries(OrderBook_run OrderBook_lib)

add_subdirectory(GoogleTest)
