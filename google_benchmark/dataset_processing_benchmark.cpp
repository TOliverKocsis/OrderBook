#include <benchmark/benchmark.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "order.hpp"
#include "order_book.hpp"
#include "order_utilities.hpp"

/*  This Google Benchmark file is for measuring the simulated dataset processing
 * run time. */

/*
 * Load the simulated traffic: order messages from a the .csv file created by
 * the data_generator.py to a vector.
 */
std::vector<OrderMessage> LoadOrdersFromCSV(const std::string& filename) {
    std::vector<OrderMessage> order_messages;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string line;
        std::getline(file, line);  // skip the header

        while (std::getline(file, line)) {
            std::string word;
            std::stringstream ss(line);
            OrderMessage next_order_msg;
            std::string order_message_type_str;

            std::getline(ss, order_message_type_str, ',');
            if (order_message_type_str == "CancelOrder") {
                // For cancel order we fill the Order msg type and order id,
                // other fields will not be used.
                next_order_msg.order_message_type = OrderMessageType::CANCEL_ORDER;

                std::getline(ss, word, ',');
                next_order_msg.order.orderId = std::stoi(word);
            } else if (order_message_type_str == "AddOrder") {
                next_order_msg.order_message_type = OrderMessageType::ADD_ORDER;

                // Fill out the Order part of the Order Message struct.
                std::getline(ss, word, ',');
                next_order_msg.order.orderId = std::stoi(word);

                std::getline(ss, word, ',');
                next_order_msg.order.order_type = StringToOrderType(word);

                std::getline(ss, word, ',');
                next_order_msg.order.price = std::stoi(word);

                std::getline(ss, word, ',');
                next_order_msg.order.quantity = std::stoi(word);
            } else if (order_message_type_str == "GetBestBid") {
                next_order_msg.order_message_type = OrderMessageType::GET_BEST_BID;
            } else if (order_message_type_str == "GetAskVolumeBetweenPrices") {
                next_order_msg.order_message_type = OrderMessageType::GET_ASK_VOLUME_BETWEEN_PRICES;

                do {
                    std::getline(ss, word, ',');  // skip empty lines, todo: not ideal
                } while (word.empty());
                next_order_msg.lower_price = std::stoi(word);
                std::getline(ss, word, ',');
                next_order_msg.upper_price = std::stoi(word);
            }

            order_messages.push_back(next_order_msg);
        }
        file.close();
    } else {
        std::cout << "Example Dataset File open failed " << filename << std::endl;
    }

    return order_messages;
}

/*
 *  Benchmark simulated dataset processing, with message feed reading.
 *  Later when multithreading is included, this might work faster.
 */
static void BM_LoadAndExecuteMessages_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        OrderBook order_book;
        benchmark::DoNotOptimize(order_book);
        std::vector<OrderMessage> order_message_feed =
            LoadOrdersFromCSV("../../example_order_dataset/example_dataset.csv");

        for (OrderMessage next_order_msg : order_message_feed) {
            if (next_order_msg.order_message_type == OrderMessageType::CANCEL_ORDER) {
                order_book.CancelOrderbyId(next_order_msg.order.orderId);
            } else if (next_order_msg.order_message_type == OrderMessageType::ADD_ORDER) {
                order_book.AddOrder(next_order_msg.order);
            }
            // Make sure compiler does not optimize out these unused values by
            // volatile flag
            else if (next_order_msg.order_message_type == OrderMessageType::GET_BEST_BID) {
                volatile std::pair<uint32_t, uint32_t> my_pair = order_book.GetBestBidWithQuantity();
            } else if (next_order_msg.order_message_type == OrderMessageType::GET_ASK_VOLUME_BETWEEN_PRICES) {
                volatile uint32_t askVolume =
                    order_book.GetVolumeBetweenPrices(next_order_msg.lower_price, next_order_msg.upper_price);
            }
        }
    }
}

BENCHMARK(BM_LoadAndExecuteMessages_SingleThread);