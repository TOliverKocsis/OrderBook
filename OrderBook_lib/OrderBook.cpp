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
                //run until we order in the orderbook, and some were maching,
                //here we have no more levels to match
                break;
            }
        }

    }


OrderBook::OrderBook(){
    // Constructor implementation
}


void OrderBook::AddOrder(Order order){
    if (order.ordertype == OrderType::buy){
        bids_db[order.orderId] = order; //add to hashmap for quick lookup and modification
        int price = order.price;
        int orderid = order.orderId;  //uniq identifier of this order
        bids_level.push(std::make_pair(price, orderid)); //add pair of [pricepoint, orderId] quick lookup of best price
    }
    if (order.ordertype == OrderType::sell){
        asks_db[order.orderId] = order;
        asks_level.push(std::make_pair(order.price, order.orderId));
    }
        //after adding new pricepoint, run processing to see if we can fulfill any orders
        ProcessOrders();
}

void printTrade(const Trade& trade) {
        std::cout << "Trade executed: BuyOrderID: " << trade.buyOrderId
                  << " with SellOrderID: " << trade.sellOrderId
                  << " at price " << trade.price
                  << " for quantity " << trade.quantity << std::endl;
    }

void OrderBook::ExecuteTrade(uint32_t buyOrderId, uint32_t sellOrderId, double price, uint32_t quantity) {
        Trade trade = {buyOrderId, sellOrderId, price, quantity, std::chrono::system_clock::now()};
        trades.push_back(trade);
        printTrade(trade);
    }

std::vector<Trade>& OrderBook::getTrades(){return trades;}
