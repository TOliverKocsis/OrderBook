#include <benchmark/benchmark.h>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <random>

/*  This Google Benchmark file is for measuring separate function execution times.
 */

// todo micro benchmarking:
// - get best bid
// - get volume between prices

/*
 *  Micro Benchmark Addorder1: adding the same order
 *  For the prioQ database version, adding the same price is probably slower, so have more test where
 *  the price changes.
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
 *  Micro Benchmark Addorder2: adding an order with random prices every iter, with range of prices in 3
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
 *  Micro Benchmark Addorder3: Increase the price range to 10
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
 *  Micro Benchmark Addorder4: Increase the price range to 10
 *  Order db has 10k size at the benchmark start.
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
 *  Micro Benchmark Cancel Order 1: Adding 1 Order, then cancel a RANDOM order, while having 1k orders in the db
 *
 */
static void BM_Cancel1_Random_Order_DB1k(benchmark::State &state) {
    OrderBook orderBook;
    std::vector<int> OrderIDs(1000, 0);  //save the id to know which one to cancel

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(99, 101);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range
    std::uniform_int_distribution<> randomIDindex(0, OrderIDs.size() - 1); // define the range

    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    //preload 1k orders, so that we actually have to look to cancel a random order
    for(int i=0; i< 1000; i++) {
        orderBook.AddOrder(buyOrder);
        OrderIDs[i]=orderID;

        //update id and price
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
 *  Micro Benchmark Cancel Order 2: Adding 1 Order, then cancel a RANDOM order, while having 10k orders in the db
 */
static void BM_Cancel1_Random_Order_DB10k(benchmark::State &state) {
    OrderBook orderBook;
    std::vector<int> OrderIDs(10000, 0);  //save the id to know which one to cancel

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(99, 101);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range
    std::uniform_int_distribution<> randomIDindex(0, OrderIDs.size() - 1); // define the range

    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    //preload 10k orders, so that we actually have to look to cancel a random order
    for(int i=0; i< 10000; i++) {
        orderBook.AddOrder(buyOrder);
        OrderIDs[i]=orderID;

        //update id and price
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
 *  Micro Benchmark Cancel Order 3: Adding 1 Order, then cancel a RANDOM order, while having 100k orders in the db
 */
static void BM_Cancel1_Random_Order_DB100k(benchmark::State &state) {
    OrderBook orderBook;
    std::vector<int> OrderIDs(100000, 0);  //save the id to know which one to cancel

    // Random number generators
    std::random_device rd;  // Random seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> priceDistrib(99, 101);  // Price range: keep stock price stable: RANGE: 3
    std::uniform_int_distribution<> quantityDistrib(50, 5000); // Define quantity range
    std::uniform_int_distribution<> randomIDindex(0, OrderIDs.size() - 1); // define the range

    int orderID = 1;
    Order buyOrder = {OrderType::buy, 1, 100, 5};

    //preload 100k orders, so that we actually have to look to cancel a random order
    for(int i=0; i< 100000; i++) {
        orderBook.AddOrder(buyOrder);
        OrderIDs[i]=orderID;

        //update id and price
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

BENCHMARK(BM_AddOrder_same_price);
BENCHMARK(BM_AddOrder_RandomPrice_Range_3);
BENCHMARK(BM_AddOrder_RandomPrice_Range_10);
BENCHMARK(BM_AddOrder_RandomPrice_Range_10_DB10k);
BENCHMARK(BM_Cancel1_Random_Order_DB1k);
BENCHMARK(BM_Cancel1_Random_Order_DB10k);
BENCHMARK(BM_Cancel1_Random_Order_DB100k);


BENCHMARK_MAIN();
