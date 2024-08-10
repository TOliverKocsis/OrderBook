//
// Created by oliverk on 06/08/24.
//

#include "gtest/gtest.h"
#include "Order.hpp"
#include "OrderBook.hpp"


TEST(ProcessOrdersTestSuit, ExactBuyAndSell) {
    /* Case1: Test exact price matching trade with same amounts.
     * Buy Order ID 1 shall match Sell Order ID 2 at price 100 for 5 units.
     */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order sellorder1{OrderType::sell, 2, 100, 5};

    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(sellorder1);

    std::vector<Trade> expectedTrades = {
        {1, 2, 100, 5, /* timestamp not compared */}
    };

    const std::vector<Trade>& actualTrades = orderBook.getTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}


TEST(ProcessOrdersTestSuit, ExactBuyAndSellDifferentAmount) {
    /* Case2: Test exact price matching trade with DIFFERENT amounts.
     * Buy Order ID 1 shall match Sell Order ID 2 at price 105 for 7 units, with 3 leftover
     * in the orderbook
     */

    Order buyorder1{OrderType::buy, 1, 105, 7};
    Order sellorder1{OrderType::sell, 2, 105, 10};

    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(sellorder1);

    //we expect the trade at 105, but only for 7 stocks
    std::vector<Trade> expectedTrades = {
        {1, 2, 105, 7, /* timestamp not compared */}
    };

    const std::vector<Trade>& actualTrades = orderBook.getTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}


TEST(ProcessOrdersTestSuit, BuyOrderForNewAndRemainingSell) {
    /* Case 3: Test a buy order(6) that matches a new and a remaining sell order(4 and 5).
    * Buy Order ID 6 shall match Sell Order ID 4 remainder:3 at price:105, and Sell Order ID 5 (new)
    * at price 102, for 3 units, totaling the trade of 6 (3+3).
    * So three trades should happen:
    * 1. Trade same as previous test, buy id:3, sell id:4 price:105, quantity:7
    * 2. Trade buy id:6, sell id:4, price: 110, quantity: 3
    * 3. Trade buy id:6, sell id:5, price: 110, quantity: 3
    * Remainder: buy id 6, 6 units, price:110
     */

    Order buyorder_3{OrderType::buy, 3, 105, 7};
    Order sellorder_4{OrderType::sell, 4, 105, 10};

    Order sellorder_5{OrderType::sell, 5, 102, 3};
    Order buyorder_6{OrderType::buy, 6, 110, 12};

    OrderBook orderBook;
    orderBook.AddOrder(buyorder_3);
    orderBook.AddOrder(sellorder_4);
    orderBook.AddOrder(sellorder_5);
    orderBook.AddOrder(buyorder_6);


    std::vector<Trade> expectedTrades = {
        {3, 4, 105, 7, /* timestamp not compared */},
        {6, 5, 110, 3, /* timestamp not compared */},
        {6, 4, 110, 3, /* timestamp not compared */}
    };
    //Note: if the buy order is at 110, it doesnt matter that the sell order was for 102 and 105,
    // the buy price is used, not sure if this applies to real world.
    // Sell order id 5 happens before 4, because it has a lower price, and lower price is checked first
    // in the current implementation.

    const std::vector<Trade>& actualTrades = orderBook.getTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}


TEST(ProcessOrdersTestSuit, SellOrderFulfilledForRemainder) {
    /* Case4: Test if a sell order is fulfilled from a new and
     * remainder buy order.
     * Trades that happen:
     * Trade 1: fullfills sell order 1: buy id:2, sell id:1, price 120 quantity 10,
     * closes sellorder1.
     * Sellorder 4 causes two trades to happen: (buyorder3 first as it ahs higher price)
     * Trade2: buy id3, sell id4, price:130, quantity 10
     * now the remainder oif buyorder 2 of 20 quantity
     * Trade3: buy id2, sell id4, price:120, quantity 20
     *
     * And a last trade just for 1 qauntity:
     * Trade 5: buyid:5 sell id4, price 119, quantity1 (exact match price)
     *
     */

    Order sellorder1{OrderType::sell, 1, 100, 10};
    Order buyorder2{OrderType::buy, 2, 120, 30}; //will have 20 remainder buy
    Order buyorder3{OrderType::buy, 3, 130, 10}; //10 new quantity, overall 30 buy
    Order sellorder4{OrderType::sell, 4, 119, 100};
    Order buyorder5{OrderType::buy, 5, 119, 1};




    OrderBook orderBook;
    orderBook.AddOrder(sellorder1);
    orderBook.AddOrder(buyorder2);
    orderBook.AddOrder(buyorder3);
    orderBook.AddOrder(sellorder4);
    orderBook.AddOrder(buyorder5);



    std::vector<Trade> expectedTrades = {
        {2, 1, 120, 10, /* timestamp not compared */},
        {3, 4, 130, 10, /* timestamp not compared */},
        {2, 4, 120, 20, /* timestamp not compared */},
        {5, 4, 119, 1, /* timestamp not compared */},
    };

    const std::vector<Trade>& actualTrades = orderBook.getTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}

//TODO: Add some negative test, when we want to sell, but buy order are too low price, and no trade is hapening for some time



