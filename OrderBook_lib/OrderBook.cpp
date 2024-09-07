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
    //todo could this be done by another thread
    while(!bids_level.empty() and !asks_level.empty()){
        uint32_t best_bid = GetBestBid();
        uint32_t best_ask = GetBestAsk();
        if (best_bid >= best_ask){
            Level& bidLevel = bids_level.begin()->second;
            Level& askLevel = asks_level.begin()->second;
            Order& bidOrder = bids_level.begin()->second.ordersList.front();
            Order& askOrder = asks_level.begin()->second.ordersList.front();

            //handle amounts traded
            uint32_t traded_amount = std::min(bidOrder.quantity, askOrder.quantity);

            //reduce quantity of trade of both ask and bid, and their Level
            uint32_t new_bid_quantity = bidOrder.quantity - traded_amount;
            bidLevel.quantity -= traded_amount;
            bidOrder.quantity=new_bid_quantity;

            uint32_t new_ask_quantity = askOrder.quantity - traded_amount;
            askLevel.quantity -= traded_amount;
            askOrder.quantity=new_ask_quantity;

            //simulate order processing
            ExecuteTrade(bidOrder.orderId, askOrder.orderId, askOrder.price, traded_amount);

            //remove the order from the orderbook, that has no quantity left
            if (bidOrder.quantity == 0){
                bids_db.erase(bidOrder.orderId);  //1. remove from hashmap
                bidLevel.ordersList.pop_front();  //2. remove from linked list
                if(bidLevel.quantity < 1) {       //3. remove empty Level from map
                    bids_level.erase(bidLevel.price);
                }
            }
            if (askOrder.quantity == 0){
                asks_db.erase(askOrder.orderId);  //1. remove from hashmap
                askLevel.ordersList.pop_front();  //2. remove from linked list
                if(askLevel.quantity < 1) {       //3. remove empty Level from map
                    asks_level.erase(askLevel.price);
                }
            }
        }
        else{break;}//no orders to match
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
        uint32_t price = order.price;
        if (order.ordertype == OrderType::buy){
            if(!bids_level.contains(price)) {
                Level newPriceLevel;    //price is new, so add price level to bin search tree (map)
                newPriceLevel.price = price;
                bids_level[price] = newPriceLevel;
            }
            bids_level[price].quantity += order.quantity;
            order.parentLevel = &bids_level[price];
            //Note: measured insert vs emplace_back and push_back, insert ~30 ns faster
            auto it = bids_level[price].ordersList.insert(bids_level[price].ordersList.end(), order);
            bids_db[order.orderId] = it;
        }
        if (order.ordertype == OrderType::sell){
            if(!asks_level.contains(price)) {
                Level newPriceLevel;    //price is new, so add price level to bin search tree (map)
                newPriceLevel.price = price;
                asks_level[price] = newPriceLevel;
            }
            asks_level[price].quantity += order.quantity;
            order.parentLevel = &asks_level[price];
            auto it = asks_level[price].ordersList.insert(asks_level[price].ordersList.end(), order);
            asks_db[order.orderId] = it;
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
            auto listIt = bids_db[orderId];     //get list iterator from hashmap
            Order& delTargetOrder = *listIt;                   //dereference it to get the Order struct
            Level& refLevel = *delTargetOrder.parentLevel;     //get a Level pointer from Order struct
            refLevel.ordersList.erase(listIt);               //remove from linkedlist pointer(=list::iterator)
            bids_db.erase(orderId);
            refLevel.quantity -= delTargetOrder.quantity;      //reduce quantity
            if(refLevel.quantity < 1) {
                bids_level.erase(refLevel.price);              //remove empty Level from map
            }
            return;
        }
        if(asks_db.contains(orderId)) {
            auto listIt = asks_db[orderId];     //get list iterator from hashmap
            Order& delTargetOrder = *listIt;                   //dereference it to get the Order struct
            Level& refLevel = *delTargetOrder.parentLevel;     //get a Level pointer from Order struct
            refLevel.ordersList.erase(listIt);               //remove from linkedlist pointer(=list::iterator)
            asks_db.erase(orderId);
            refLevel.quantity -= delTargetOrder.quantity;      //reduce quantity
            if(refLevel.quantity < 1) {
                asks_level.erase(refLevel.price);              //remove empty Level from map
            }
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
    if(bids_level.empty()) {return std::make_pair(0, 0);}
    return std::make_pair(bids_level.begin()->first, bids_level.begin()->second.quantity);
}

/*
 * Return pair of 1:Price and 2:Quantity
 * If there are multiple bids on the same price (same level) their quantites are added together
 */
std::pair<uint32_t, uint32_t> OrderBook::GetBestAskWithQuantity(){
    if(asks_level.empty()) {return std::make_pair(0, 0);}
    return std::make_pair(asks_level.begin()->first, asks_level.begin()->second.quantity);
}


/*
 * Returns the quantity of ask orders between start and end input values both being inclusive
 */
uint32_t OrderBook::GetVolumeBetweenPrices(uint32_t start, uint32_t end) {
    if(asks_level.empty()) {return 0;}
    if(start > end) {return 0;}
    //if the first ask price, so lowest, is already lower than end value, quantity will be zero
    if(asks_level.begin()->first > end){return 0;}

    uint32_t volume = 0;
    for(uint32_t curPriceCheck=start; curPriceCheck<end+1; curPriceCheck++) {
        if(asks_level.contains(curPriceCheck)){volume+= asks_level[curPriceCheck].quantity;}
    }
    return volume;
}

unsigned long OrderBook::GetBidQuantity() {
    unsigned long quantity=0;
    for(auto const& mapPair: bids_level) {
        quantity += mapPair.second.quantity;
    }
    return quantity;
}

unsigned long OrderBook::GetAskQuantity() {
    unsigned long quantity=0;
    for(auto const& mapPair: asks_level) {
        quantity += mapPair.second.quantity;
    }
    return quantity;
}

uint32_t OrderBook::GetBestBid() {
    if(bids_level.empty()){return 0;}
    //todo Note: would it be faster if we only delete a empty levels if there are more then 20 empty levels for example?
    //this would cause to be forced to skip some empty levels when searching for the next bid, making code bit more complex
    //and with extra branching it would be slower for most cases?
    //leaving empty levels would only help the cases when we keep adding then deleting levels
    /*while(!bids_level.empty() and bids_level.begin()->second.quantity==0) {
        //so while the map is not empty, and the top has zero quantity, purge the Level, as it causes slow down?
        //or is it better to just always skip the empty levels?
        bids_level.erase(bids_level.begin());
    }*/
    return bids_level.begin()->second.price;
}

uint32_t OrderBook::GetBestAsk() {
    if(asks_level.empty()){return 0;}
    return asks_level.begin()->second.price;
}
