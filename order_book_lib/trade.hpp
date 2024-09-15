//
// Created by oliverk on 08/08/24.
//
#ifndef TRADE_HPP
#define TRADE_HPP
#include <chrono>

struct Trade {
    uint32_t buyOrderId = -1;
    uint32_t sellOrderId = -1;
    double price = -1;
    uint32_t quantity = -1;
    std::chrono::system_clock::time_point timestamp;

    // Overload == to ignore time for testcases
    bool operator==(const Trade& other) const {
        return buyOrderId == other.buyOrderId && sellOrderId == other.sellOrderId && price == other.price &&
               quantity == other.quantity;
    }
};

#endif  // TRADE_HPP