#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP
#include <cstdint>  // defines uint32 type
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Trade.hpp"
#include "level.hpp"
#include "order.hpp"

class order_book {
   private:
    std::unordered_map<uint32_t, std::list<order>::iterator> bids_db_;  // orderid -> Order struct in Levels std::list
    std::unordered_map<uint32_t, std::list<order>::iterator> asks_db_;

    std::map<uint32_t, level, std::greater<>> bids_level_;  // price -> level object of orders in list
    std::map<uint32_t, level> asks_level_;

    std::vector<Trade> trades;  // simulate and record trades, used for testing

    uint32_t order_id_tracker_;

   public:
    order_book();

    // prevent OrderBook copying and moving
    order_book(const order_book&) = delete;
    void operator=(const order_book&) = delete;
    order_book(order_book&&) = delete;
    void operator=(order_book&&) = delete;

    void AddOrder(order order);
    void CancelOrderbyId(uint32_t order_id);
    void ProcessOrders();
    void ExecuteTrade(uint32_t buy_order_id, uint32_t sellOrderId, double price, uint32_t quantity);
    std::vector<Trade>& GetTrades();
    std::pair<uint32_t, uint32_t> GetBestBidWithQuantity();
    std::pair<uint32_t, uint32_t> GetBestAskWithQuantity();
    uint32_t GetBestBid();
    uint32_t GetBestAsk();
    uint32_t GetVolumeBetweenPrices(uint32_t start, uint32_t end);
    unsigned long GetBidQuantity();
    unsigned long GetAskQuantity();
};

#endif  // ORDERBOOK_HPP
