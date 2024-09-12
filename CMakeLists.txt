cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 20)

# Define the macro for regular builds
# add_compile_definitions(ENABLE_DEBUG_PRINTS) #todo bug: being used for google benchmark runs also

set(SOURCE_FILES main.cpp)
add_executable(OrderBook_run ${SOURCE_FILES})

include_directories(order_book_lib)
add_subdirectory(order_book_lib)

target_link_libraries(OrderBook_run OrderBook_lib)

add_subdirectory(GoogleTest)
add_subdirectory(google_benchmark)
