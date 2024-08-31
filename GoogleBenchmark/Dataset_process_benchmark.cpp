#include <benchmark/benchmark.h>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <random>
#include <fstream>
#include <iostream>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>


/*  This Google Benchmark file is for measuring the simulated dataset processing run time. */


/* Helper */
OrderType stringToOrderType(const std::string& str) {
    if (str == "buy") return OrderType::buy;
    if (str == "sell") return OrderType::sell;
    return OrderType::undefined;
}

/*
 * Load the simulated traffic: order messages from a the .csv file created by the DataGenerator.py to a vector
 *
 * todo: When multi threading added, use a ring queue for continues read write, that would solve memory bottleneck.
 *  If the size of queue would be fixed, it would be only written when there is space in it, instead of filling
 *  memory until it can, and then start processing.
 * todo: known issue: this loads everything to the memory, that might get full and crash if the csv file is too big
 */
std::vector<OrderMessage> loadOrdersFromCSV(const std::string& filename) {
    std::vector<OrderMessage> orderMessages;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::cout << "Example Dataset opened " << filename << std::endl;
        std::string line;
        // skip the header
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::string word;
            std::stringstream ss(line);
            OrderMessage nextOrderMsg;
            std::string orderMessageTypeStr;

            std::getline(ss, orderMessageTypeStr, ',');
            if(orderMessageTypeStr == "CancelOrder") {
                //for cancel order we fill the Order msg type and order id, other fields will not be used
                nextOrderMsg.orderMessageType = OrderMessageType::cancelOrder;

                std::getline(ss, word, ',');
                nextOrderMsg.order.orderId = std::stoi(word);
            }
            else if(orderMessageTypeStr == "AddOrder") {
                nextOrderMsg.orderMessageType = OrderMessageType::addOrder;

                //fill out the Order part of the Order Message struct
                std::getline(ss, word, ',');
                nextOrderMsg.order.orderId = std::stoi(word);

                std::getline(ss, word, ',');
                nextOrderMsg.order.ordertype = stringToOrderType(word);

                std::getline(ss, word, ',');
                nextOrderMsg.order.price = std::stoi(word);

                std::getline(ss, word, ',');
                nextOrderMsg.order.quantity = std::stoi(word);
            }
            else if(orderMessageTypeStr == "GetBestBid") {
                nextOrderMsg.orderMessageType = OrderMessageType::getBestBid;
            }
            else if(orderMessageTypeStr == "GetAskVolumeBetweenPrices") {
                nextOrderMsg.orderMessageType = OrderMessageType::getAskVolumeBetweenPrices;

                /* todo:
                 * Since we use a static placement opf variables, but GetAsk.. does not have most of the values,
                 * we need to skip multiple fields, until we get to the end. This is not ideal, and the csv could be
                 * separated into diff csv files, but anyway real data planned be used, so decided to not focus on this
                 * for now*/
                do {
                    std::getline(ss, word, ',');  // skip empty lines
                }while(word.empty());
                nextOrderMsg.lowerPrice = std::stoi(word);
                std::getline(ss, word, ',');
                nextOrderMsg.upperPrice = std::stoi(word);
            }

            orderMessages.push_back(nextOrderMsg);
        }
        file.close();
    }
    else {
        std::cout << "Example Dataset File open failed " << filename << std::endl;
    }

    return orderMessages;
}

/*
 *  Benchmark simulated dataset processing, with message feed reading.
 *  Later when multithreading is included, this might work faster.
 */
static void BM_LoadAndExecuteMessages_singlethread(benchmark::State &state) {

    for (auto _: state) {
        OrderBook orderBook;
        benchmark::DoNotOptimize(orderBook);
        std::vector<OrderMessage> orderMessageFeed =
            loadOrdersFromCSV("../../ExampleOrderDataset/ExampleDataset.csv");

        for(OrderMessage nextOrderMsg : orderMessageFeed) {
            if(nextOrderMsg.orderMessageType == OrderMessageType::cancelOrder) {
                orderBook.CancelOrderbyId(nextOrderMsg.order.orderId);
            }
            else if(nextOrderMsg.orderMessageType == OrderMessageType::addOrder) {
                orderBook.AddOrder(nextOrderMsg.order);
            }
            // Make sure compiler does not optimize out these unused values by volatile flag
            else if(nextOrderMsg.orderMessageType == OrderMessageType::getBestBid) {
                volatile std::pair<uint32_t, uint32_t> mypair = orderBook.GetBestBidWithQuantity();
            }
            else if(nextOrderMsg.orderMessageType == OrderMessageType::getAskVolumeBetweenPrices) {
                volatile uint32_t askVolume =
                    orderBook.GetVolumeBetweenPrices(nextOrderMsg.lowerPrice, nextOrderMsg.upperPrice);
            }
        }
    }
}

BENCHMARK(BM_LoadAndExecuteMessages_singlethread);