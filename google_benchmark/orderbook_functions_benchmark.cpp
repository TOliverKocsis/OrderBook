#include <benchmark/benchmark.h>

#include <random>

#include "order.hpp"
#include "order_book.hpp"

/*  This Google Benchmark file is for measuring separate function execution times. */

/*
 *  Benchmark AddOrder:
 *  Measure Asymptotic Complexity of Adding an order with random prices, with random prices range of 10,
 *  while having N Orders already added to OrderBook.
 */
static void BM_AddOrder_PriceRange_3(benchmark::State &state) {
    OrderBook order_book;
    int order_id = 1;
    Order buy_order = {OrderType::BUY, 1, 100, 5};

    std::random_device rd;                                                        // random seed
    std::mt19937 gen(rd());                                                       // mersenne Twister engine
    std::uniform_int_distribution<> uniform_int_distribution_price(99, 101);      // price range: 3
    std::uniform_int_distribution<> uniform_int_distribution_quantity(50, 5000);  // quantity range

    // preload Orderbook DB
    for (int i = 0; i < state.range(0); i++) {
        order_book.AddOrder(buy_order);
        order_id++;
        buy_order.orderId = order_id;
        buy_order.price = uniform_int_distribution_price(gen);
        buy_order.quantity = uniform_int_distribution_quantity(gen);
    }

    for (auto _ : state) {
        buy_order.orderId = ++order_id;
        buy_order.price = uniform_int_distribution_price(gen);
        buy_order.quantity = uniform_int_distribution_quantity(gen);
        order_book.AddOrder(buy_order);
        benchmark::DoNotOptimize(order_book);
    }
    state.SetComplexityN(state.range(0));
}

/*
 *  Benchmark AddOrder:
 *  Measure Asymptotic Complexity of Adding an order with random prices, with random prices range of 10,
 *  while having N Orders already added to OrderBook.
 */
static void BM_AddOrder_PriceRange_20(benchmark::State &state) {
    OrderBook order_book;
    int order_id = 1;
    Order buy_order = {OrderType::BUY, 1, 100, 5};

    std::random_device rd;                                      // random seed
    std::mt19937 gen(rd());                                     // mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(95, 114);      // price range increased to 20
    std::uniform_int_distribution<> quantityDistrib(50, 5000);  // define quantity range

    // preload Orderbook DB
    for (int i = 0; i < state.range(0); i++) {
        order_book.AddOrder(buy_order);
        order_id++;
        buy_order.orderId = order_id;
        buy_order.price = priceDistrib(gen);
        buy_order.quantity = quantityDistrib(gen);
    }

    for (auto _ : state) {
        buy_order.orderId = ++order_id;
        buy_order.price = priceDistrib(gen);
        buy_order.quantity = quantityDistrib(gen);
        order_book.AddOrder(buy_order);
        benchmark::DoNotOptimize(order_book);
    }
    state.SetComplexityN(state.range(0));
}

/*
 *  Benchmark Cancel Order Asymptotic Complexity:
 *  Measure Asymptotic Complexity of adding 1 order, then cancel a RANDOM order, while having N preloaded amount
 *  of Orders. Reason to add an order is to not run out of Orders during test execution.
 */
static void BM_Add1_Cancel1_Random_Order(benchmark::State &state) {
    OrderBook order_book;
    std::vector<unsigned long> order_ids(state.range(0), 0);  // save the id to know which one to cancel

    std::random_device rd;                                                        // random seed
    std::mt19937 gen(rd());                                                       // mersenne Twister engine
    std::uniform_int_distribution<> uniform_int_distribution_price(100, 119);     // price range : 20
    std::uniform_int_distribution<> uniform_int_distribution_quantity(50, 5000);  // define quantity range
    std::uniform_int_distribution<> random_id_index(0, order_ids.size() - 1);     // define the range

    int order_id = 1;
    Order buy_order = {OrderType::BUY, 1, 100, 5};

    // preload varied amount of orders, based on benchmark state input
    for (int i = 0; i < state.range(0); i++) {
        order_book.AddOrder(buy_order);
        order_ids[i] = order_id;
        order_id++;
        buy_order.orderId = order_id;
        buy_order.price = uniform_int_distribution_price(gen);
        buy_order.quantity = uniform_int_distribution_quantity(gen);
    }

    for (auto _ : state) {
        // Note that there is an added overhead of pausing and resuming timer.
        state.PauseTiming();
        buy_order.orderId = ++order_id;
        order_book.AddOrder(buy_order);  // keep adding orders so that we keep the 10k size
        // Get random order id, add the new order id to its place to avoid deleting from middle of vector
        // would be O(n)
        int random_index = random_id_index(gen);
        unsigned long random_order_id = order_ids[random_index];
        order_ids[random_index] = order_id;
        state.ResumeTiming();

        order_book.CancelOrderbyId(random_order_id);
        benchmark::DoNotOptimize(order_book);
    }

    state.SetComplexityN(state.range(0));
}

static void BM_Add1_Cancel1_Random_Order_Sell(benchmark::State &state) {
    OrderBook order_book;
    std::vector<unsigned long> order_ids(state.range(0), 0);  // save the id to know which one to cancel

    std::random_device rd;                                                        // random seed
    std::mt19937 gen(rd());                                                       // mersenne Twister engine
    std::uniform_int_distribution<> uniform_int_distribution_price(100, 119);     // price range : 20
    std::uniform_int_distribution<> uniform_int_distribution_quantity(50, 5000);  // define quantity range
    std::uniform_int_distribution<> random_id_index(0, order_ids.size() - 1);     // define the range

    int order_id = 1;
    Order sell_order = {OrderType::SELL, 1, 100, 5};

    // preload varied amount of orders, based on benchmark state input
    for (int i = 0; i < state.range(0); i++) {
        order_book.AddOrder(sell_order);
        order_ids[i] = order_id;
        order_id++;
        sell_order.orderId = order_id;
        sell_order.price = uniform_int_distribution_price(gen);
        sell_order.quantity = uniform_int_distribution_quantity(gen);
    }

    for (auto _ : state) {
        // Added overhead of pausing and resuming timer
        state.PauseTiming();
        sell_order.orderId = ++order_id;
        order_book.AddOrder(sell_order);  // keep adding orders so that we keep the 10k size
        // Get random order id, add the new order id to its place to avoid deleting from middle of vector (would be
        // O(n)) int randomIndex = std::rand() % OrderIDs.size();
        int random_index = random_id_index(gen);
        int random_order_id = order_ids[random_index];
        order_ids[random_index] = order_id;
        state.ResumeTiming();

        order_book.CancelOrderbyId(random_order_id);
        benchmark::DoNotOptimize(order_book);
    }

    state.SetComplexityN(state.range(0));
}

/*
 *  Benchmark Get Best Bid:
 *  Measure Asymptotic Complexity of get best bid price and quantity of it, while having N orders in the Orderbook.
 */
static void BM_GetBestBid(benchmark::State &state) {
    OrderBook order_book;

    std::random_device rd;                                                        // random seed
    std::mt19937 gen(rd());                                                       // mersenne Twister engine
    std::uniform_int_distribution<> uniform_int_distribution_price(98, 100);      // price range: 3
    std::uniform_int_distribution<> uniform_int_distribution_quantity(50, 5000);  // define quantity range

    int order_id = 1;
    Order buy_order = {OrderType::BUY, 1, 100, 5};

    // preload varied amount of orders, based on benchmark state input
    for (int i = 0; i < state.range(0); i++) {
        order_book.AddOrder(buy_order);
        order_id++;
        buy_order.orderId = order_id;
        buy_order.price = uniform_int_distribution_price(gen);
        buy_order.quantity = uniform_int_distribution_quantity(gen);
    }

    for (auto _ : state) {
        // Benchmark loop
        benchmark::DoNotOptimize(order_book);
        order_book.GetBestBidWithQuantity();
    }
    state.SetComplexityN(state.range(0));
}

/*
 *  Benchmark Get Ask Volume Between Prices
 *  Measure Asymptotic Complexity of Get Ask Volume Between Prices, while having N orders in the Orderbook.
 */
static void BM_GetAskVolumeBetweenPrices(benchmark::State &state) {
    OrderBook order_book;

    // Random number generators
    std::random_device rd;                                              // random seed
    std::mt19937 gen(rd());                                             // mersenne Twister engine
    std::uniform_int_distribution<> uniform_int_distribution(98, 100);  // price range: 3
    std::uniform_int_distribution<> int_distribution(50, 5000);         // define quantity range

    int order_id = 1;
    Order ask_order = {OrderType::SELL, 1, 100, 5};

    // preload varied amount of orders, based on benchmark state input
    for (int i = 0; i < state.range(0); i++) {
        order_book.AddOrder(ask_order);
        order_id++;
        ask_order.orderId = order_id;
        ask_order.price = uniform_int_distribution(gen);
        ask_order.quantity = int_distribution(gen);
    }

    for (auto _ : state) {
        // Benchmark loop
        order_book.GetVolumeBetweenPrices(99, 100);
        benchmark::DoNotOptimize(order_book);
    }
    state.SetComplexityN(state.range(0));
}

// Add Order Benchmarks
BENCHMARK(BM_AddOrder_PriceRange_3)->RangeMultiplier(2)->Range(1 << 10, 1 << 20)->Complexity();
BENCHMARK(BM_AddOrder_PriceRange_20)->RangeMultiplier(2)->Range(1 << 10, 1 << 20)->Complexity();

// Cancel Order Benchmarks
BENCHMARK(BM_Add1_Cancel1_Random_Order)->RangeMultiplier(2)->Range(1 << 10, 1 << 20)->Complexity();
BENCHMARK(BM_Add1_Cancel1_Random_Order_Sell)->RangeMultiplier(2)->Range(1 << 10, 1 << 20)->Complexity();

// Get Best Bid Benchmarks
BENCHMARK(BM_GetBestBid)->RangeMultiplier(2)->Range(1 << 10, 1 << 20)->Complexity();

// Get Ask Volume between Prices Benchmarks
BENCHMARK(BM_GetAskVolumeBetweenPrices)->RangeMultiplier(2)->Range(1 << 10, 1 << 20)->Complexity();


// Init and run all BENCHMARK macro registered cases
BENCHMARK_MAIN();
