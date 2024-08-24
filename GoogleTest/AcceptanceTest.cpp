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

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
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

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
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

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
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

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}

TEST(ProcessOrdersTestSuit, LevelsDontMatch) {
    /* Test when price levels dont match, no trades should happen
    *  Add multiple orders, but only one trade should happen
    */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order buyorder2{OrderType::buy, 2, 99, 5};
    Order buyorder3{OrderType::buy, 3, 98, 99};
    Order buyorder4{OrderType::buy, 4, 1, 1};

    Order sellorder5{OrderType::sell, 5, 100, 5};
    Order sellorder6{OrderType::sell, 6, 100, 5};
    Order sellorder7{OrderType::sell, 7, 101, 99};
    Order sellorder8{OrderType::sell, 8, 1000, 1};


    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(buyorder2);
    orderBook.AddOrder(buyorder3);
    orderBook.AddOrder(buyorder4);
    orderBook.AddOrder(sellorder5);
    orderBook.AddOrder(sellorder6);
    orderBook.AddOrder(sellorder7);
    orderBook.AddOrder(sellorder8);

    std::vector<Trade> expectedTrades = {
        {1, 5, 100, 5, /* timestamp not compared */}
    };

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }

}

TEST(ProcessOrdersTestSuit, IncorrectInput) {
    /* Test that orders with incorrect input throws and error when added to orderbook,
     *  and it does not execute trades.
     */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order buyorder2{OrderType::buy, 2, 101, 0};     //zero quantity
    Order buyorder3{OrderType::undefined, 3, 200, 5};     //undefined type
    Order buyorder4{OrderType::buy, 0, 103, 5};     //zero orderid
    Order buyorder6{OrderType::buy, 1, 105, 5};     //already existing orderid
    Order buyorder7{OrderType::buy, 7, 0, 5};       //zero price
    Order buyorder9{OrderType::buy, 9, 99, 5};     //corect order just no match
    Order sellorder1{OrderType::sell, 11, 100, 5};
    Order sellorder2{OrderType::sell, 12, 200, 0};      //zero quantity
    Order sellorder4{OrderType::sell, 0, 200, 5};       //zero orderid
    Order sellorder6{OrderType::sell, 11, 200, 5};      //already existing orderid
    Order sellorder7{OrderType::sell, 17, 0, 5};        //zero price
    Order sellorder9{OrderType::sell, 19, 200, 100};      //corect order just no match

    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    EXPECT_THROW(orderBook.AddOrder(buyorder2), std::invalid_argument);
    orderBook.AddOrder(buyorder3);
    EXPECT_THROW(orderBook.AddOrder(buyorder4), std::invalid_argument);
    EXPECT_THROW(orderBook.AddOrder(buyorder6), std::invalid_argument);
    EXPECT_THROW(orderBook.AddOrder(buyorder7), std::invalid_argument);
    orderBook.AddOrder(buyorder9);

    orderBook.AddOrder(sellorder1);
    EXPECT_THROW(orderBook.AddOrder(sellorder2), std::invalid_argument);
    EXPECT_THROW(orderBook.AddOrder(sellorder4), std::invalid_argument);
    EXPECT_THROW(orderBook.AddOrder(sellorder6), std::invalid_argument);
    EXPECT_THROW(orderBook.AddOrder(sellorder7), std::invalid_argument);
    orderBook.AddOrder(sellorder9);

    std::vector<Trade> expectedTrades = {
        {1, 11, 100, 5, /* timestamp not compared */}
    };

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
    //ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}

TEST(ProcessOrdersTestSuit, CancelOneOrder) {
    /*
     * Make an order, but cancel it, then a matching order comes, but since we cancel no trade shall happen
     */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order sellorder1{OrderType::sell, 2, 100, 5};

    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.CancelOrderbyId(buyorder1.orderId);
    orderBook.AddOrder(sellorder1);
    orderBook.CancelOrderbyId(sellorder1.orderId);

    std::vector<Trade> expectedTrades = {};

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}

TEST(ProcessOrdersTestSuit, MultipleOrdersCancelOneOrder) {
    /*
    *  Add multiple orders, but only one trade could happen, that is canceled before the matching order,
    *  therefore no trades should happen
    */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order buyorder2{OrderType::buy, 2, 99, 5};
    Order buyorder3{OrderType::buy, 3, 98, 99};
    Order buyorder4{OrderType::buy, 4, 1, 1};

    Order sellorder5{OrderType::sell, 5, 100, 5};
    Order sellorder6{OrderType::sell, 6, 100, 5};
    Order sellorder7{OrderType::sell, 7, 101, 99};
    Order sellorder8{OrderType::sell, 8, 1000, 1};


    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(buyorder2);
    orderBook.AddOrder(buyorder3);
    orderBook.AddOrder(buyorder4);
    orderBook.CancelOrderbyId(buyorder1.orderId);

    orderBook.AddOrder(sellorder5);
    orderBook.AddOrder(sellorder6);
    orderBook.AddOrder(sellorder7);
    orderBook.AddOrder(sellorder8);

    std::vector<Trade> expectedTrades = {};

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }
}

TEST(ProcessOrdersTestSuit, getBestBidTest) {
    /*
     *  Checks if GetBestBidWithQuantity function returns correct bid price and quantity
     */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order buyorder2{OrderType::buy, 2, 100, 5};
    Order buyorder3{OrderType::buy, 3, 100, 5};
    Order buyorder4{OrderType::buy, 4, 100, 5};
    Order buyorder5{OrderType::buy, 5, 100, 5};
    Order buyorder6{OrderType::buy, 6, 100, 15}; //price: 100, quantity: 40 overall
    Order buyorder7{OrderType::buy, 7, 99, 1000};
    Order buyorder8{OrderType::buy, 7, 99, 1000};

    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(buyorder2);
    orderBook.AddOrder(buyorder3);
    orderBook.AddOrder(buyorder4);
    orderBook.AddOrder(buyorder5);
    orderBook.AddOrder(buyorder6);
    orderBook.AddOrder(buyorder7);

    std::pair<int, int> bidinfo =  orderBook.GetBestBidWithQuantity();
    std::pair<int, int> expectedBidinfo = {100, 40};

    EXPECT_EQ(bidinfo, expectedBidinfo);
}

TEST(ProcessOrdersTestSuit, getBestBidTestWithOnlyOnePrice) {
    /*
     *  Checks if GetBestBidWithQuantity function returns correct bid price and quantity
     *  difference to previous test, there is only one level in the orderbook
     *  to test if we are not trying to access a prioq (or other datastructures) that is empty
     *  and causing segfault.
     */

    Order buyorder1{OrderType::buy, 1, 100, 5};
    Order buyorder2{OrderType::buy, 2, 100, 5};
    Order buyorder3{OrderType::buy, 3, 100, 5};
    Order buyorder4{OrderType::buy, 4, 100, 5};
    Order buyorder5{OrderType::buy, 5, 100, 5};
    Order buyorder6{OrderType::buy, 6, 100, 15}; //price: 100, quantity: 40 overall


    OrderBook orderBook;
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(buyorder2);
    orderBook.AddOrder(buyorder3);
    orderBook.AddOrder(buyorder4);
    orderBook.AddOrder(buyorder5);
    orderBook.AddOrder(buyorder6);


    std::pair<int, int> bidinfo =  orderBook.GetBestBidWithQuantity();
    std::pair<int, int> expectedBidinfo = {100, 40};

    EXPECT_EQ(bidinfo, expectedBidinfo);
}

TEST(ProcessOrdersTestSuit, getAskVolumeBetweenPrices) {
    /*
     *  Checks if GetVolumeBetweenPrices function returns correct ask quantity between two price points
     */

    Order sellorder1{OrderType::sell, 1, 10, 5};
    Order sellorder2{OrderType::sell, 2, 11, 10};
    Order sellorder3{OrderType::sell, 3, 12, 5};

    OrderBook orderBook;
    orderBook.AddOrder(sellorder1);
    orderBook.AddOrder(sellorder2);
    orderBook.AddOrder(sellorder3);

    int askQuantityInfo =  orderBook.GetVolumeBetweenPrices(10, 11);
    int expectedAskQuantityInfo = 15;

    EXPECT_EQ(askQuantityInfo, expectedAskQuantityInfo);
}

TEST(ProcessOrdersTestSuit, getAskVolumeBetweenPrices_WrongInput) {
    /*
     *  Checks if GetVolumeBetweenPrices function returns zero in case of wrong inputs
     */

    Order sellorder1{OrderType::sell, 1, 10, 5};
    Order sellorder2{OrderType::sell, 2, 11, 10};
    Order sellorder3{OrderType::sell, 3, 12, 5};

    OrderBook orderBook;
    orderBook.AddOrder(sellorder1);
    orderBook.AddOrder(sellorder2);
    orderBook.AddOrder(sellorder3);

    int askQuantityInfo =  orderBook.GetVolumeBetweenPrices(11, 10); //start>end wrong input
    int expectedAskQuantityInfo = 0;

    EXPECT_EQ(askQuantityInfo, expectedAskQuantityInfo);
}


TEST(ProcessOrdersTestSuit, getAskVolumeBetweenPrices_BottomOfList) {
    /*
     *  Checks if GetVolumeBetweenPrices return correct volume if we ask for an amount at the bottom of the list
     */

    Order sellorder1{OrderType::sell, 1, 10, 5};
    Order sellorder2{OrderType::sell, 2, 11, 10};
    Order sellorder3{OrderType::sell, 3, 12, 5};

    OrderBook orderBook;
    orderBook.AddOrder(sellorder1);
    orderBook.AddOrder(sellorder2);
    orderBook.AddOrder(sellorder3);

    int askQuantityInfo =  orderBook.GetVolumeBetweenPrices(12, 12); //start>end wrong input
    int expectedAskQuantityInfo = 5;

    EXPECT_EQ(askQuantityInfo, expectedAskQuantityInfo);
}


//todo also check if the pushing back all the orders was correct
TEST(ProcessOrdersTestSuit, getAskVolumeBetweenPrices_OrdersCorrectAfter) {
    /*
     *  Checks if GetVolumeBetweenPrices return correct volume if we ask for an amount at the bottom of the list,
     *  and that orders were correctly pushed in afterward, and trades happen as expected.
     *  Also check here when the start price is smaller than the lowest ask.
     */

    Order sellorder1{OrderType::sell, 1, 10, 10};
    Order sellorder2{OrderType::sell, 2, 10, 10};
    Order sellorder3{OrderType::sell, 3, 12, 5};
    Order sellorder4{OrderType::sell, 4, 12, 5};
    Order sellorder5{OrderType::sell, 5, 13, 5};

    OrderBook orderBook;
    orderBook.AddOrder(sellorder1);
    orderBook.AddOrder(sellorder2);
    orderBook.AddOrder(sellorder3);
    orderBook.AddOrder(sellorder4);
    orderBook.AddOrder(sellorder5);

    //Check 3 different prices
    int askQuantityInfo =  orderBook.GetVolumeBetweenPrices(13, 13); //start>end wrong input
    int expectedAskQuantityInfo = 5;
    EXPECT_EQ(askQuantityInfo, expectedAskQuantityInfo);

    int askQuantityInfo2 =  orderBook.GetVolumeBetweenPrices(12, 12); //start>end wrong input
    int expectedAskQuantityInfo2 = 10;
    EXPECT_EQ(askQuantityInfo2, expectedAskQuantityInfo2);

    int askQuantityInfo3 =  orderBook.GetVolumeBetweenPrices(5, 15); //start>end wrong input
    int expectedAskQuantityInfo3 = 35;
    EXPECT_EQ(askQuantityInfo3, expectedAskQuantityInfo3);

    //check if trade happens as expected after asking volumes
    Order buyorder1{OrderType::buy, 6, 10, 100};
    orderBook.AddOrder(buyorder1);

    std::vector<Trade> expectedTrades = {
        {6, 1, 10, 10, /* timestamp not compared */},
        {6, 2, 10, 10, /* timestamp not compared */}
    };

    const std::vector<Trade>& actualTrades = orderBook.GetTrades();
    ASSERT_EQ(expectedTrades.size(), actualTrades.size());
    //check every field in the structure if it is the same
    for (size_t i = 0; i < expectedTrades.size(); ++i) {
        EXPECT_EQ(expectedTrades[i], actualTrades[i]);
    }

}


