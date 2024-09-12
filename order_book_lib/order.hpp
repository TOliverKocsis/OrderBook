#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>  // uint32 type

#include "level.hpp"

enum class OrderType {
    UNDEFINED,
    BUY,
    SELL,
};

// forward declaration of level for parentLevel*
struct Level;

struct Order {
    OrderType order_type{OrderType::UNDEFINED};
    uint32_t orderId{};
    uint32_t price{};
    uint32_t quantity{};
    Level *parent_level{nullptr};
    std::list<Order>::iterator listPosition;
};

enum class OrderMessageType { UNDEFINED, ADD_ORDER, CANCEL_ORDER, GET_BEST_BID, GET_ASK_VOLUME_BETWEEN_PRICES };

// To handle ExampleDataset.csv lines
struct OrderMessage {
    OrderMessageType order_message_type{OrderMessageType::UNDEFINED};
    Order order;
    // Add fields for GetBestBid and GetAskVolumeBetweenPrices
    int lower_price{0};  // For GetAskVolumeBetweenPrices
    int upper_price{0};  // For GetAskVolumeBetweenPrices
};

#endif  // ORDER_HPP
