cmake_minimum_required(VERSION 3.28)

project(Google_benchmark)
add_subdirectory(benchmark)

# adding the benchmark run target
add_executable(Google_Benchmark_run OrderBook_benchmark.cpp)

# linking Google_benchmark with OrderBook which will be tested
target_link_libraries(Google_Benchmark_run OrderBook_lib)

# linking Google Benchmark
target_link_libraries(Google_Benchmark_run benchmark::benchmark)
