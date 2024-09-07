//
// Created by oliverk on 29/07/24.
//

#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP
#include <cstdint> // defines uint32 type
#include <map>
#include <vector>
#include <unordered_map>
#include <utility>
#include "Order.hpp"
#include "Trade.hpp"
#include "Level.hpp"

class OrderBook{

private:
    std::unordered_map<uint32_t, std::list<Order>::iterator> bids_db; //orderid -> Order struct in Levels std::list
    std::unordered_map<uint32_t, std::list<Order>::iterator> asks_db;

    std::map<uint32_t, Level, std::greater<>> bids_level; //price -> Level object of orders organized into a linked list
    std::map<uint32_t, Level> asks_level;

    std::vector<Trade> trades;  //simulate and record trades, used for testing

    uint32_t OrderIDTracker;

public:
    OrderBook();

    //prevent OrderBook copying and moving
    OrderBook(const OrderBook&) = delete;
    void operator=(const OrderBook&) = delete;
    OrderBook(OrderBook&&) = delete;
    void operator=(OrderBook&&) = delete;

    void AddOrder(Order order);
    void CancelOrderbyId(uint32_t orderId);
    void ProcessOrders();
    void ExecuteTrade(uint32_t buyOrderId, uint32_t sellOrderId, double price, uint32_t quantity);
    std::vector<Trade>& GetTrades();
    std::pair<uint32_t, uint32_t> GetBestBidWithQuantity();
    std::pair<uint32_t, uint32_t> GetBestAskWithQuantity();
    uint32_t GetBestBid();
    uint32_t GetBestAsk();
    uint32_t GetVolumeBetweenPrices(uint32_t start, uint32_t end);
    unsigned long GetBidQuantity();
    unsigned long GetAskQuantity();

};

#endif //ORDERBOOK_HPP
