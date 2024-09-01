#include <benchmark/benchmark.h>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <random>

/*  This Google Benchmark file is for measuring separate function execution times. */

/*
 *  Benchmark AddOrder:
 *  Measure Asymptotic Complexity of Adding an order with random prices, with random prices range of 10,
 *  while having N Orders already added to OrderBook.
 */
static void BM_AddOrder_PriceRange_3(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(99, 101);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    //preload Orderbook DB
    for(int i=0; i< state.range(0); i++) {
        orderBook.AddOrder(buyOrder);
        //update id and price
        orderID++;
        buyOrder.orderId = orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
    }

    for (auto _: state) {
        buyOrder.orderId = ++orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
        orderBook.AddOrder(buyOrder);
        benchmark::DoNotOptimize(orderBook);
    }
    state.SetComplexityN(state.range(0));
}


/*
 *  Benchmark AddOrder:
 *  Measure Asymptotic Complexity of Adding an order with random prices, with random prices range of 10,
 *  while having N Orders already added to OrderBook.
 */
static void BM_AddOrder_PriceRange_10(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(95, 104);  // Price range: RANGE increased to 10
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    //preload Orderbook DB
    for(int i=0; i< state.range(0); i++) {
        orderBook.AddOrder(buyOrder);

        //update id and price
        orderID++;
        buyOrder.orderId = orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
    }

    for (auto _: state) {
        buyOrder.orderId = ++orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
        orderBook.AddOrder(buyOrder);
        benchmark::DoNotOptimize(orderBook);
    }
    state.SetComplexityN(state.range(0));
}


/*
 *  Benchmark Cancel Order Asymptotic Complexity:
 *  Measure Asymptotic Complexity of adding 1 Order, then cancel a RANDOM order, while having N preloaded amount
 *  of Orders. Reason to add an order is to not run out of Orders during test execution.
 *
 */
static void BM_Add1_Cancel1_Random_Order(benchmark::State &state) {
    OrderBook orderBook;
    std::vector<int> OrderIDs(state.range(0), 0);  //save the id to know which one to cancel

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(99, 101);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range
    std::uniform_int_distribution<> randomIDindex(0, OrderIDs.size() - 1); // define the range

    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    //preload varied amount of orders, based on benchmark state input
    for(int i=0; i< state.range(0); i++) {
        orderBook.AddOrder(buyOrder);
        OrderIDs[i]=orderID;
        orderID++;
        buyOrder.orderId = orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
    }


    for (auto _: state) {
        //Added overhead of pausing and resuming timer, could be ignored as the Cancel order time is awful at the moment
        state.PauseTiming();
        buyOrder.orderId = ++orderID;
        orderBook.AddOrder(buyOrder); //keep adding orders so that we keep the 10k size
        //Get random order id, add the new order id to its place to avoid deleting from middle of vector (would be O(n))
        //int randomIndex = std::rand() % OrderIDs.size();
        int randomIndex = randomIDindex(gen);
        int randomOrderID = OrderIDs[randomIndex];
        OrderIDs[randomIndex] = orderID;
        state.ResumeTiming();

        orderBook.CancelOrderbyId(randomOrderID);
        benchmark::DoNotOptimize(orderBook);
    }

    state.SetComplexityN(state.range(0));
}


/*
 *  Benchmark Get Best Bid:
 *  Measure Asymptotic Complexity of get best bid price and quantity of it, while having N orders in the Orderbook.
 */
static void BM_GetBestBid_bid(benchmark::State &state) {
    OrderBook orderBook;

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(98, 100);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    //preload varied amount of orders, based on benchmark state input
    for(int i=0; i< state.range(0); i++) {
        orderBook.AddOrder(buyOrder);
        orderID++;
        buyOrder.orderId = orderID;
        buyOrder.price = priceDistrib(gen);
        buyOrder.quantity = quantityDistrib(gen);
    }

    for (auto _: state) {
        //Benchmark loop
        benchmark::DoNotOptimize(orderBook);
        orderBook.GetBestBidWithQuantity();
    }
    state.SetComplexityN(state.range(0));
}


/*
 *  Benchmark Get Ask Volume Between Prices
 *  Measure Asymptotic Complexity of Get Ask Volume Between Prices, while having N orders in the Orderbook.
 */
static void BM_GetAskVolumeBetweenPrices(benchmark::State &state) {
    OrderBook orderBook;

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(98, 100);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    int orderID = 1;
    Order askOrder = {OrderType::sell, 1, 100, 5};

    //preload varied amount of orders, based on benchmark state input
    for(int i=0; i< state.range(0); i++) {
        orderBook.AddOrder(askOrder);
        orderID++;
        askOrder.orderId = orderID;
        askOrder.price = priceDistrib(gen);
        askOrder.quantity = quantityDistrib(gen);
    }

    for (auto _: state) {
        //Benchmark loop
        orderBook.GetVolumeBetweenPrices(99, 100);
        benchmark::DoNotOptimize(orderBook);
    }
    state.SetComplexityN(state.range(0));
}


//Add Order Benchmarks
BENCHMARK(BM_AddOrder_PriceRange_3)->RangeMultiplier(2)->Range(1<<10, 1<<18)->Complexity();
BENCHMARK(BM_AddOrder_PriceRange_10)->RangeMultiplier(2)->Range(1<<10, 1<<18)->Complexity();

//Cancel Order Benchmarks
BENCHMARK(BM_Add1_Cancel1_Random_Order)->RangeMultiplier(2)->Range(1<<10, 1<<18)->Complexity();

//Get Best Bid Benchmarks
BENCHMARK(BM_GetAskVolumeBetweenPrices)->RangeMultiplier(2)->Range(1<<10, 1<<18)->Complexity();

//Get Ask Volume between Prices Benchmarks
BENCHMARK(BM_GetAskVolumeBetweenPrices)->RangeMultiplier(2)->Range(1<<10, 1<<18)->Complexity();

//Init and run all BENCHMARK macro registered cases
BENCHMARK_MAIN();
