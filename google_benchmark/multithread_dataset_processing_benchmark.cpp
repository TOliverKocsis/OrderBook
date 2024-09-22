#include <benchmark/benchmark.h>

#include <string>
#include <thread>

#include "dataset_process.hpp"

static void BM_LoadAndExecuteMessages_MultiThread(benchmark::State& state) {
    filename = "../../example_order_dataset/example_dataset.csv";

    for (auto _ : state) {
        read_in_is_done = false;  // flag for consumer_thread to keep running
        std::thread producer_thread{LoadOrdersFromCSV};
        std::thread consumer_thread{ProcessOrderMessages};
        producer_thread.join();
        consumer_thread.join();
    }
}

BENCHMARK(BM_LoadAndExecuteMessages_MultiThread);
