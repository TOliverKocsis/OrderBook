//
// Created by oliverk on 29/07/24.
//

#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP
#include <cstdint> // defines uint32 type
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <utility>
#include "Order.hpp"
#include "Trade.hpp"

class OrderBook{

private:
    std::unordered_map<uint32_t, Order> bids_db; //orderid -> Order struct
    std::unordered_map<uint32_t, Order> asks_db;

    std::priority_queue<std::pair<int, int>> bids_level; //first:price second:orderid
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<> > asks_level; //price -> orderid, min heap

    std::vector<Trade> trades;  //simulate and record trades, used for testing

    uint32_t OrderIDTracker;

public:


    OrderBook();

    // prevent copying and moving
    //Orderbook(const OrderBook&) = delete;
    //void operator=(const OrderBook&) = delete;
    //OrderBook(OrderBook&&) = delete;
    //void operator=(OrderBook&&) = delete;

    void AddOrder(Order order);
    void CancelOrderbyId(uint32_t orderId);
    void ProcessOrders();
    void ExecuteTrade(uint32_t buyOrderId, uint32_t sellOrderId, double price, uint32_t quantity);
    std::vector<Trade>& GetTrades();
    std::pair<uint32_t, uint32_t> GetBestBidWithQuantity();

};

#endif //ORDERBOOK_HPP
