#include <string>

#include "order.hpp"
#include "order_book.hpp"

#ifndef ORDER_UTILITIES_HPP
#define ORDER_UTILITIES_HPP

inline OrderType StringToOrderType(const std::string& str) {
    if (str == "buy") return OrderType::BUY;
    if (str == "sell") return OrderType::SELL;
    return OrderType::UNDEFINED;
}

#endif  // ORDER_UTILITIES_HPP
