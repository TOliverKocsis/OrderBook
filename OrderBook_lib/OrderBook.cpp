//
// Created by oliverk on 29/07/24.
//
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <utility>
#include "Order.hpp"
#include "OrderBook.hpp"

#include <stack>
#include <utility>

// Preprocessor macro definitions
#ifdef ENABLE_DEBUG_PRINTS
    #define DEBUG_PRINT(x) std::cout << x << std::endl;
#else
    #define DEBUG_PRINT(x) do {} while (0)
#endif

void OrderBook::ProcessOrders(){

        //TODO could this be done by another thread?
        while(!bids_level.empty() and !asks_level.empty()){
            auto best_bid = bids_level.top();
            auto best_ask = asks_level.top();
            if (best_bid.first >= best_ask.first){
                //fulfill the order if levels are in line

                Order& bid_order = bids_db[best_bid.second];
                Order& ask_order = asks_db[best_ask.second];

                //handle amounts traded
                int traded_amount = std::min(bid_order.quantity, ask_order.quantity);

                //reduce quantity of trade of both ask and bid by the smaller amount
                uint32_t new_bid_quantity = bid_order.quantity - traded_amount;
                bid_order.quantity=new_bid_quantity;

                uint32_t new_ask_quantity = ask_order.quantity - traded_amount;
                ask_order.quantity=new_ask_quantity;

                //simulate order processing
                ExecuteTrade(bid_order.orderId, ask_order.orderId, bid_order.price, traded_amount);

                //remove the order from the orderbook, that has no quantity left
                if (bid_order.quantity == 0){
                    bids_db.erase(bid_order.orderId);
                    bids_level.pop();
                }
                if (ask_order.quantity == 0){
                    asks_db.erase(ask_order.orderId);
                    asks_level.pop();
                }
            }
            else{
                //run until we order in the orderbook, and some were matching,
                //here we have no more levels to match
                break;
            }
        }
    }


OrderBook::OrderBook(){
        // Constructor implementation
        OrderIDTracker = 0; //track max order id so far, to keep the rule of increasing order numbers during a day
}


void OrderBook::AddOrder(Order order){
        if(order.quantity < 1) {
            throw std::invalid_argument("Quantity must be more than zero.");
        }
        if(order.orderId <= OrderIDTracker) {
            throw std::invalid_argument("Order ID must be increasing and uniq number.");
        }
        if(order.price < 1) {
            throw std::invalid_argument("Price must be more than zero.");
        }
        if(order.ordertype == OrderType::undefined){return;}  //chose to simply ignore undefined orders

        OrderIDTracker = std::max(OrderIDTracker, order.orderId);
        if (order.ordertype == OrderType::buy){
            bids_db[order.orderId] = order; //add to hashmap for quick lookup and modification
            uint32_t price = order.price;
            uint32_t orderid = order.orderId;  //uniq identifier of this order
            bids_level.push(std::make_pair(price, orderid)); //add pair of [pricepoint, orderId] for quick lookup of best price
        }
        if (order.ordertype == OrderType::sell){
            asks_db[order.orderId] = order;
            asks_level.push(std::make_pair(order.price, order.orderId));
        }
        //after adding new price point, run processing to see if we can fulfill any orders
        ProcessOrders();
}


/*
 * Cancel an order based on order id
 */
void OrderBook::CancelOrderbyId(uint32_t orderId) {
        //check if we can see this orderid in either asks or bids
        if(bids_db.contains(orderId)) {
            //todo this is O(N+logN), not ideal,  O(1) avg cancellation would be preferred
            //where N is the number of orders in the db(memory), deeper the order in the book is worse
            //todo if we have some constant level info that needs to be updated

            //make a stack and dig out the order we need from the prioQ, then put them back
            std::stack<std::pair<int,int>> bids_stack;
            auto bid = bids_level.top();
            bids_level.pop();
            bids_stack.push(bid);

            while(bids_stack.top().second != orderId) {
                bid = bids_level.top();
                bids_level.pop();
                bids_stack.push(bid);
            }
            //found bid, and its on stack top now, already removed from prioq
            bids_stack.pop(); //removed now
            while(!bids_stack.empty()) {
                //re add all the orders to the prioq
                bid = bids_stack.top();
                bids_stack.pop();
                bids_level.push(bid);
            }
            bids_db.erase(orderId);
            return;
        }

        if(asks_db.contains(orderId)) {
            //make a stack and dig out the order we need from the prioQ
            std::stack<std::pair<int,int>> asks_stack;
            auto ask = asks_level.top();
            asks_level.pop();
            asks_stack.push(ask);

            while(asks_stack.top().second != orderId) {
                ask = asks_level.top();
                asks_level.pop();
                asks_stack.push(ask);
            }
            //found bid, and its on stack top now, already removed from prioq
            asks_stack.pop(); //removed now
            while(!asks_stack.empty()) {
                //re add all the orders to the prioq
                ask = asks_stack.top();
                asks_stack.pop();
                asks_level.push(ask);
            }
            asks_db.erase(orderId);
        }
}


void printTrade(const Trade& trade) {
    //debug print controlled by Cmake flag to disable print during benchmark(/"release")
        DEBUG_PRINT("Trade executed: BuyOrderID: " << trade.buyOrderId
                  << " with SellOrderID: " << trade.sellOrderId
                  << " at price " << trade.price
                  << " for quantity " << trade.quantity << std::endl);
    }


void OrderBook::ExecuteTrade(uint32_t buyOrderId, uint32_t sellOrderId, double price, uint32_t quantity) {
        Trade trade = {buyOrderId, sellOrderId, price, quantity, std::chrono::system_clock::now()};
        trades.push_back(trade);
        printTrade(trade);
    }


std::vector<Trade>& OrderBook::GetTrades(){return trades;}


/*
 * Return pair of Price and Quantity
 * If there are multiple bids on the same price (same level) their quantites are added together
 */
std::pair<uint32_t, uint32_t> OrderBook::GetBestBidWithQuantity(){

    int bidPrice = bids_level.top().first;
    int bidLevelQuantity = 0;

    //now need to check if we have more quantity on this level
    //again a not ideal way, we need to pop from the tree O(logN) and put it to the stack until we see same prices
    //but go ahead with implementation for measuring performance, and comparing it to better implementations
    std::stack<std::pair<int,int>> bidsStackSave;

    while(!bids_level.empty() and bids_level.top().first == bidPrice) {
        auto nextBid = bids_level.top();
        bids_level.pop();
        bidsStackSave.push(nextBid);
        bidLevelQuantity += bids_db[nextBid.second].quantity;
    }
    //the bids_level prioq top is now different price, or empty
    //now push back all the stack
    while(!bidsStackSave.empty()) {
        auto nextBid = bidsStackSave.top();
        bidsStackSave.pop();
        bids_level.push(nextBid); //push back to prioQ to keep previous state
    }
    return std::make_pair(bidPrice, bidLevelQuantity);
}


/*
 * Returns the quantity of ask orders between start and end input values both being inclusive
 */
uint32_t OrderBook::GetVolumeBetweenPrices(uint32_t start, uint32_t end) {
    //todo again this is very inefficient: O(nlogn) time and O(N) space where N is the depth in the ask prioq
    uint32_t volume = 0;
    if(start > end) {return 0;}
    //if the first ask price, so lowest, is already lower than end value, quantity will be zero
    if(asks_level.top().first > end) {return 0;}

    std::stack<std::pair<int,int>> asksStackSave;
    while(!asks_level.empty() and start > asks_level.top().first) { //BUG todo what if start value is much smaller??
        //save until we arrive to the price we do want to process
        auto nextAsk = asks_level.top();
        asks_level.pop();
        asksStackSave.push(nextAsk);
    }
    //if we ended up searching until asks level is empty, we put back from stack and ret
    if(asks_level.empty()) {
        while(!asksStackSave.empty()) {
            auto nextAskSave = asksStackSave.top();
            asksStackSave.pop();
            asks_level.push(nextAskSave);
        }
        return 0;
    }
    //if prioq is not empty and prev while loop stopped it means we are at a price where current top = start
    //so we search until the price is out of range, or prioq empty
    while(!asks_level.empty() and asks_level.top().first <= end) {
        auto nextAsk = asks_level.top();
        asks_level.pop();
        asksStackSave.push(nextAsk);
        volume += asks_db[nextAsk.second].quantity;
    }
    //put back examined orders
    while(!asksStackSave.empty()) {
        auto nextAskSave = asksStackSave.top();
        asksStackSave.pop();
        asks_level.push(nextAskSave);
    }

    return volume;
}
