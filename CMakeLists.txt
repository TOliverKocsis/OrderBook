cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 20)

# Check for Debug build type
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_DEBUG_PRINTS)
endif ()

set(SOURCE_FILES main.cpp)
add_executable(OrderBook_run ${SOURCE_FILES})

include_directories(order_book_lib)
add_subdirectory(order_book_lib)

target_link_libraries(OrderBook_run OrderBook_lib)

add_subdirectory(google_test)
add_subdirectory(google_benchmark)
