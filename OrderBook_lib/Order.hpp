//
// Created by oliverk on 29/07/24.
//

#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint> // defines uint32 type

enum class OrderType
{
    undefined,
    buy,
    sell,
};

struct Order {
    OrderType ordertype{OrderType::undefined};
    uint32_t orderId{};
    uint32_t price{};
    uint32_t quantity{};
};

enum class OrderMessageType
{
    undefined,
    addOrder,
    cancelOrder,
};

struct OrderMessage {
    OrderMessageType orderMessageType{OrderMessageType::undefined};
    Order order;
};


#endif //ORDER_HPP
