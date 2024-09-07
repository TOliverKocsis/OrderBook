//
// Created by oliverk on 29/07/24.
//

#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint> // defines uint32 type

#include "Level.hpp"

enum class OrderType
{
    undefined,
    buy,
    sell,
};

//forward declaration of Level for parentLevel*
struct Level;


struct Order {
    OrderType ordertype{OrderType::undefined};
    uint32_t orderId{};
    uint32_t price{};
    uint32_t quantity{};
    Level* parentLevel{nullptr};
    std::list<Order>::iterator listPosition;
};

enum class OrderMessageType
{
    undefined,
    addOrder,
    cancelOrder,
    getBestBid,
    getAskVolumeBetweenPrices
};

// To handle ExampleDataset.csv lines
struct OrderMessage {
    OrderMessageType orderMessageType{OrderMessageType::undefined};
    Order order;
    // Add fields for GetBestBid and GetAskVolumeBetweenPrices
    int lowerPrice{0};  // For GetAskVolumeBetweenPrices
    int upperPrice{0};  // For GetAskVolumeBetweenPrices
};


#endif //ORDER_HPP
