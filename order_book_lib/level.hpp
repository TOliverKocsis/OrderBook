#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <list>

#include "order.hpp"

/* Llevel is an object for a price level of an instrument. It encapsulates every standing order for the instrument at
 * this price point. Internally it holds a bidirectional linked list of order_list object.
 */

struct Order;

struct Level {
    uint32_t quantity{};
    uint32_t price{};
    std::list<Order> orders_list{};
};

#endif  // LEVEL_HPP
