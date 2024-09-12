#ifndef TRADE_HPP
#define TRADE_HPP
#include <chrono>

struct trade {
    uint32_t buy_order_id = -1;
    uint32_t sell_order_id = -1;
    double price = -1;
    uint32_t quantity = -1;
    std::chrono::system_clock::time_point timestamp;

    // Overload == to ignore time for testcases
    bool operator==(const trade& other) const {
        return buy_order_id == other.buy_order_id && sell_order_id == other.sell_order_id && price == other.price &&
               quantity == other.quantity;
    }
};

#endif  // TRADE_HPP
