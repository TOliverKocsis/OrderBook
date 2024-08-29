#include <benchmark/benchmark.h>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <random>

/*  This Google Benchmark file is for measuring separate function execution times.
 */


/*
 *  Micro Benchmark Addorder Test 1:
 *  Adding the same order. For the prioQ database version, adding the same price is probably slower,
 *  so have more test where the price changes.
 *  Order db empty at start.
 */
static void BM_AddOrder_same_price(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    for (auto _: state) {
        //Pausing and resuming timing seem to have a huge overhead compared to AddOrder that ruins result
        //state.PauseTiming();
        buyOrder.orderId = ++orderID;
        benchmark::DoNotOptimize(orderBook);
        //state.ResumeTiming();

        orderBook.AddOrder(buyOrder);
    }
}


/*
 *  Micro Benchmark Addorder Test 2:
 *  Adding an order with random prices every iter, with range of prices in 3.
 *  Order db empty at start.
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
 *  Micro Benchmark Addorder Test 3:
 *  Increase the price range to 10.
 *  Order db empty at start.
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


/*
 *  Micro Benchmark Addorder Test 4:
 *  Increase the price range to 10 Order db has 10k size at the benchmark start.
 */
static void BM_AddOrder_RandomPrice_Range_10_DB10k(benchmark::State &state) {
    OrderBook orderBook;
    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(95, 104);  // Price range: RANGE increased to 10
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range

    //preload Orderbook DB
    for(int i=0; i< 1000; i++) {
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
}


/*
 *  Micro Benchmark Cancel Order:
 *  Adding 1 Order, then cancel a RANDOM order, while having a preloaded amount
 *  of Orders. The amount changes with test input benchmark::state: 100 1k 10k 100k amount of preloaded orders in the
 *  db, before we start adding, then canceling a random Order.
 */
static void BM_Cancel1_Random_Order_DB_(benchmark::State &state) {
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
}

/*
 *  Micro Benchmark Get Best Bid 1
 *  Price Range of preloaded messages matters, since if less orders are on present on the best level
 *  less orders needs to be collected. (At least for current implementation)
 */
static void BM_GetBestBid_bidDB_(benchmark::State &state) {
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
}


/*
 *  Micro Benchmark Get Ask Volume Between Prices
 *
 */
static void BM_GetAskVolume_askDB_(benchmark::State &state) {
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
        benchmark::DoNotOptimize(orderBook);
        orderBook.GetVolumeBetweenPrices(99, 100);
    }
}

//Add Order Benchmarks
BENCHMARK(BM_AddOrder_same_price);
BENCHMARK(BM_AddOrder_RandomPrice_Range_3);
BENCHMARK(BM_AddOrder_RandomPrice_Range_10);
BENCHMARK(BM_AddOrder_RandomPrice_Range_10_DB10k);

//Cancel Order Benchmarks
BENCHMARK(BM_Cancel1_Random_Order_DB_)->RangeMultiplier(10)->Range(100, 100000);

//Get Best Bid Benchmarks
BENCHMARK(BM_GetBestBid_bidDB_)->RangeMultiplier(10)->Range(100, 100000);

//Get Ask Volume between Prices Benchmarks
BENCHMARK(BM_GetAskVolume_askDB_)->RangeMultiplier(10)->Range(100, 100000);


BENCHMARK_MAIN();
