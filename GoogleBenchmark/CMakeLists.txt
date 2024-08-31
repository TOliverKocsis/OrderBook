cmake_minimum_required(VERSION 3.28)

project(Google_benchmark)

# Add the Google Benchmark subdirectory
add_subdirectory(benchmark)

# Specify all benchmark source files
set(    BENCHMARK_SOURCES
        OrderBook_benchmark.cpp
        Dataset_process_benchmark.cpp
)

# Adding the benchmark run target
add_executable(Google_Benchmark_run ${BENCHMARK_SOURCES})

# Link the benchmark executable with OrderBook_lib and Google Benchmark
target_link_libraries(Google_Benchmark_run OrderBook_lib)
target_link_libraries(Google_Benchmark_run benchmark::benchmark)
