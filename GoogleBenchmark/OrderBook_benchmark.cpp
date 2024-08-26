#include <benchmark/benchmark.h>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <random>


// todo micro benchmarking:
// - Addorder
// - Cancel order
// - get best bid
// - get volume between prices

/*
 *  Micro Benchmark Addorder1: adding the same order
 *  For the prioQ database version, adding the same price is probably slower, so have more test where
 *  the price changes.
 */
static void BM_AddOrder(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    for (auto _: state) {
        orderBook.AddOrder(buyOrder);
        buyOrder.orderId = ++orderID;
        benchmark::DoNotOptimize(orderBook);
    }
}

/*
 *  Micro Benchmark Addorder2: adding an order with random prices every iter
 */
static void BM_AddOrder_RandomPrice_Range_3(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(99, 101);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    for (auto _: state) {
        buyOrder.orderId = ++orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
        orderBook.AddOrder(buyOrder);

        benchmark::DoNotOptimize(orderBook);
    }
}

/*
 *  Micro Benchmark Addorder3: Increase the price range diff
 */
static void BM_AddOrder_RandomPrice_Range_10(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(95, 104);  // Price range: RANGE increased to 10
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    for (auto _: state) {
        buyOrder.orderId = ++orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
        orderBook.AddOrder(buyOrder);

        benchmark::DoNotOptimize(orderBook);
    }
}

BENCHMARK(BM_AddOrder);
BENCHMARK(BM_AddOrder_RandomPrice_Range_3);
BENCHMARK(BM_AddOrder_RandomPrice_Range_10);

BENCHMARK_MAIN();
