//
// Created by oliverk on 29/07/24.
//

#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>  // defines uint32 type

enum class OrderType {
    UNDEFINED,
    BUY,
    SELL,
};

struct Order {
    OrderType order_type{OrderType::UNDEFINED};
    uint32_t orderId{};
    uint32_t price{};
    uint32_t quantity{};
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