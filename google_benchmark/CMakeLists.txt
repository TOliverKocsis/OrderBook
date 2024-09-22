cmake_minimum_required(VERSION 3.28)

project(Google_benchmark)

# Add the Google Benchmark subdirectory
add_subdirectory(benchmark)

# Specify all benchmark source files
set(BENCHMARK_SOURCES
        orderbook_functions_benchmark.cpp
        dataset_processing_benchmark.cpp
        multithread_dataset_processing_benchmark.cpp
        ${CMAKE_SOURCE_DIR}/dataset_process.cpp
)

# Adding the benchmark run target
add_executable(Google_Benchmark_run ${BENCHMARK_SOURCES})


# Include main directory for dataset_process.hpp
target_include_directories(Google_Benchmark_run PRIVATE ${CMAKE_SOURCE_DIR})

# Link the benchmark executable with OrderBook_lib and Google Benchmark
target_link_libraries(Google_Benchmark_run OrderBook_lib)
target_link_libraries(Google_Benchmark_run benchmark::benchmark)

