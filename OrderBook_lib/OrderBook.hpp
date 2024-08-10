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
    std::unordered_map<int, Order> bids_db; //orderid -> Order struct
    std::unordered_map<int, Order> asks_db;

    std::priority_queue<std::pair<int, int>> bids_level; //first:price second:orderid
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<> > asks_level; //price -> orderid, min heap

    std::vector<Trade> trades;  //simulate and record trades, used for testing

public:


    OrderBook();

    // prevent copying and moving
    //Orderbook(const OrderBook&) = delete;
    //void operator=(const OrderBook&) = delete;
    //OrderBook(OrderBook&&) = delete;
    //void operator=(OrderBook&&) = delete;

    void AddOrder(Order order);
    void CancelOrder(Order order);
    void ProcessOrders();
    void ExecuteTrade(uint32_t buyOrderId, uint32_t sellOrderId, double price, uint32_t quantity);
    //void displayOrderbook();
    std::vector<Trade>& getTrades();

};

#endif //ORDERBOOK_HPP
