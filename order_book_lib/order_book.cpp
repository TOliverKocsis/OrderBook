#include "order_book.hpp"

#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "order.hpp"

// Preprocessor macro definitions
#ifdef ENABLE_DEBUG_PRINTS
#define DEBUG_PRINT(x) std::cout << x << std::endl;
#else
#define DEBUG_PRINT(x) \
    do {               \
    } while (false)
#endif

/*
 * Check if we can match sell and buy orders in the OrderBook for trades to happen.
 * If trade happens, delete Orders with zero quantity left.
 */
void OrderBook::ProcessOrders() {
    while (!bids_level_.empty() and !asks_level_.empty()) {
        uint32_t best_bid = GetBestBid();
        uint32_t best_ask = GetBestAsk();
        if (best_bid >= best_ask) {
            Level &bid_level = bids_level_.begin()->second;
            Level &ask_level = asks_level_.begin()->second;
            Order &bid_order = bids_level_.begin()->second.orders_list.front();
            Order &ask_order = asks_level_.begin()->second.orders_list.front();

            uint32_t traded_amount = std::min(bid_order.quantity, ask_order.quantity);

            // Reduce quantity of trade of both ask and bid, and their holding level.
            uint32_t new_bid_quantity = bid_order.quantity - traded_amount;
            bid_level.quantity -= traded_amount;
            bid_order.quantity = new_bid_quantity;

            uint32_t new_ask_quantity = ask_order.quantity - traded_amount;
            ask_level.quantity -= traded_amount;
            ask_order.quantity = new_ask_quantity;

            // Simulate order record / sending a network message.
            ExecuteTrade(bid_order.orderId, ask_order.orderId, ask_order.price, traded_amount);

            // Remove empty orders from hashmap, linked list, and purge empty level with zero orders.
            if (bid_order.quantity == 0) {
                bids_db_.erase(bid_order.orderId);  // 1. remove from hashmap
                bid_level.orders_list.pop_front();  // 2. remove from linked list
                if (bid_level.quantity < 1) {       // 3. remove empty level from map
                    bids_level_.erase(bid_level.price);
                }
            }
            if (ask_order.quantity == 0) {
                asks_db_.erase(ask_order.orderId);  // 1. remove from hashmap
                ask_level.orders_list.pop_front();  // 2. remove from linked list
                if (ask_level.quantity < 1) {       // 3. remove empty level from map
                    asks_level_.erase(ask_level.price);
                }
            }
        } else {
            break;
        }  // no orders to match
    }
}

OrderBook::OrderBook() {
    // Track max order id so far, to keep the rule of increasing order numbers during a day.
    order_id_tracker_ = 0;
    bids_db_.reserve(262145);
}

void OrderBook::AddOrder(Order order) {
    if (order.quantity < 1) {
        throw std::invalid_argument("Quantity must be more than zero.");
    }
    if (order.orderId <= order_id_tracker_) {
        throw std::invalid_argument("Order ID must be increasing and uniq number.");
    }
    if (order.price < 1) {
        throw std::invalid_argument("Price must be more than zero.");
    }
    if (order.order_type == OrderType::UNDEFINED) {
        return;  // chose to simply ignore undefined orders
    }

    order_id_tracker_ = std::max(order_id_tracker_, order.orderId);
    uint32_t price = order.price;
    if (order.order_type == OrderType::BUY) {
        if (!bids_level_.contains(price)) {
            // Add price level to bin search tree (std::map).
            Level new_price_level;
            new_price_level.price = price;
            bids_level_[price] = new_price_level;
        }
        bids_level_[price].quantity += order.quantity;
        order.parent_level = &bids_level_[price];
        auto it = bids_level_[price].orders_list.insert(bids_level_[price].orders_list.end(), order);
        bids_db_[order.orderId] = it;
    }
    if (order.order_type == OrderType::SELL) {
        if (!asks_level_.contains(price)) {
            // Add price level to bin search tree (std::map).
            Level new_price_level;
            new_price_level.price = price;
            asks_level_[price] = new_price_level;
        }
        asks_level_[price].quantity += order.quantity;
        order.parent_level = &asks_level_[price];
        auto it = asks_level_[price].orders_list.insert(asks_level_[price].orders_list.end(), order);
        asks_db_[order.orderId] = it;
    }
    // After adding new price point, run processing to see if we can fulfill any orders.
    ProcessOrders();
}

/*
 * Cancel an order based on order id.
 */
void OrderBook::CancelOrderbyId(uint32_t order_id) {
    if (bids_db_.contains(order_id)) {
        auto listIt = bids_db_[order_id];                   // get list iterator from hashmap
        Order &del_target_order = *listIt;                  // dereference it to get the Order struct
        Level &ref_level = *del_target_order.parent_level;  // get a level pointer from Order struct
        ref_level.orders_list.erase(listIt);                // remove from linkedlist pointer(=list::iterator)
        bids_db_.erase(order_id);
        ref_level.quantity -= del_target_order.quantity;  // reduce quantity
        if (ref_level.quantity < 1) {
            bids_level_.erase(ref_level.price);  // remove empty level from map
        }
        return;
    }
    if (asks_db_.contains(order_id)) {
        auto list_iterator = asks_db_[order_id];         // get list iterator from hashmap
        Order &delTargetOrder = *list_iterator;          // dereference it to get the Order struct
        Level &refLevel = *delTargetOrder.parent_level;  // get a level pointer from Order struct
        refLevel.orders_list.erase(list_iterator);       // remove from linkedlist pointer(=list::iterator)
        asks_db_.erase(order_id);
        refLevel.quantity -= delTargetOrder.quantity;  // reduce quantity
        if (refLevel.quantity < 1) {
            asks_level_.erase(refLevel.price);  // remove empty level from map
        }
    }
}

void printTrade(const trade &trade) {
    // Debug print controlled by Cmake flag to disable print during benchmark(/"release").
    DEBUG_PRINT("Trade executed: BuyOrderID: " << trade.buyOrderId << " with SellOrderID: " << trade.sellOrderId
                                               << " at price " << trade.price << " for quantity " << trade.quantity
                                               << std::endl);
}

void OrderBook::ExecuteTrade(uint32_t buy_order_id, uint32_t sellOrderId, double price, uint32_t quantity) {
    trade trade = {buy_order_id, sellOrderId, price, quantity, std::chrono::system_clock::now()};
    trades.push_back(trade);
    printTrade(trade);
}

std::vector<trade> &OrderBook::GetTrades() { return trades; }

/*
 * Return pair of Price and Quantity.
 * If there are multiple bids on the same price (same level) their quantites are
 * added together.
 */
std::pair<uint32_t, uint32_t> OrderBook::GetBestBidWithQuantity() {
    if (bids_level_.empty()) {
        return std::make_pair(0, 0);
    }
    return std::make_pair(bids_level_.begin()->first, bids_level_.begin()->second.quantity);
}

/*
 * Return pair of 1:Price and 2:Quantity.
 * If there are multiple bids on the same price (same level) their quantites are
 * added together.
 */
std::pair<uint32_t, uint32_t> OrderBook::GetBestAskWithQuantity() {
    if (asks_level_.empty()) {
        return std::make_pair(0, 0);
    }
    return std::make_pair(asks_level_.begin()->first, asks_level_.begin()->second.quantity);
}

/*
 * Returns the quantity of ask orders between start and end input values, both
 * being inclusive.
 */
uint32_t OrderBook::GetVolumeBetweenPrices(uint32_t start, uint32_t end) {
    if (asks_level_.empty()) {
        return 0;
    }
    if (start > end) {
        return 0;
    }
    // If the first ask price, so lower, is already lower than end value, quantity will be zero.
    if (asks_level_.begin()->first > end) {
        return 0;
    }

    uint32_t volume = 0;
    for (uint32_t curPriceCheck = start; curPriceCheck < end + 1; curPriceCheck++) {
        if (asks_level_.contains(curPriceCheck)) {
            volume += asks_level_[curPriceCheck].quantity;
        }
    }
    return volume;
}

unsigned long OrderBook::GetBidQuantity() {
    unsigned long quantity = 0;
    for (auto const &mapPair : bids_level_) {
        quantity += mapPair.second.quantity;
    }
    return quantity;
}

unsigned long OrderBook::GetAskQuantity() {
    unsigned long quantity = 0;
    for (auto const &mapPair : asks_level_) {
        quantity += mapPair.second.quantity;
    }
    return quantity;
}

uint32_t OrderBook::GetBestBid() {
    if (bids_level_.empty()) {
        return 0;
    }
    return bids_level_.begin()->second.price;
}

uint32_t OrderBook::GetBestAsk() {
    if (asks_level_.empty()) {
        return 0;
    }
    return asks_level_.begin()->second.price;
}
