//
// Created by oliverk on 04/09/24.
//

#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <list>
#include "Order.hpp"

/* Level is an object for a price level of an instrument. It encapsulates every standing order for the instrument at
 * this pricepoint. Internally it holds a bidirectional linked list of Order_list object.
 */

struct Order;

struct Level{
     uint32_t quantity{};
     uint32_t price{};
     std::list<Order> ordersList{};
};


#endif //LEVEL_HPP
