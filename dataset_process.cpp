#include "dataset_process.hpp"

#include <boost/lockfree/spsc_queue.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "order.hpp"
#include "order_book.hpp"
#include "order_utilities.hpp"

boost::lockfree::spsc_queue<OrderMessage> order_messages(1024);
OrderBook order_book;
std::atomic<bool> read_in_is_done;
uint32_t debug_dummy_volume_ask = 0;
uint32_t debug_dummy_volume_bid = 0;
std::string filename = "../example_order_dataset/example_dataset.csv";

// Preprocessor macro definitions
#ifdef ENABLE_DEBUG_PRINTS
#define DEBUG_PRINT(x) std::cout << x << std::endl;
#else
#define DEBUG_PRINT(x) \
    do {               \
    } while (0)
#endif


/*
 * Load the simulated traffic: order messages from a the .csv file created by the data_generator.py to a vector.
 */
void LoadOrdersFromCSV() {
    std::ifstream file(filename);

    if (file.is_open()) {
        DEBUG_PRINT("Example Dataset opened " << filename);
        std::string line;
        std::getline(file, line);  // skip the header

        while (std::getline(file, line)) {
            std::string word;
            std::stringstream ss(line);
            OrderMessage next_order_msg;
            std::string order_message_type_str;

            std::getline(ss, order_message_type_str, ',');
            if (order_message_type_str == "CancelOrder") {
                // For cancel order we fill the Order msg type and order id, other fields will not be used.
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
                    std::getline(ss, word, ',');  // skip empty lines
                } while (word.empty());
                next_order_msg.lower_price = std::stoi(word);
                std::getline(ss, word, ',');
                next_order_msg.upper_price = std::stoi(word);
            }

            order_messages.push(next_order_msg);
        }
        file.close();
    } else {
        std::cout << "Example Dataset File open failed " << filename << std::endl;
    }
    read_in_is_done = true;
}

void ProcessOrderMessages() {
    while (!read_in_is_done) {
        // check if single producer single consumer data queue has messages to consume
        if (order_messages.read_available() == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        // get next order message from spsc queu
        OrderMessage next_order_msg;
        order_messages.pop(next_order_msg);

        if (next_order_msg.order_message_type == OrderMessageType::CANCEL_ORDER) {
            order_book.CancelOrderbyId(next_order_msg.order.orderId);
        } else if (next_order_msg.order_message_type == OrderMessageType::ADD_ORDER) {
            order_book.AddOrder(next_order_msg.order);
        }
        // Make sure compiler does not optimize by volatile flag (during benchmark where no dummy).
        else if (next_order_msg.order_message_type == OrderMessageType::GET_BEST_BID) {
            volatile std::pair<int, int> my_pair = order_book.GetBestBidWithQuantity();
            debug_dummy_volume_bid += my_pair.second;
        } else if (next_order_msg.order_message_type == OrderMessageType::GET_ASK_VOLUME_BETWEEN_PRICES) {
            volatile uint32_t askVolume =
                order_book.GetVolumeBetweenPrices(next_order_msg.lower_price, next_order_msg.upper_price);
            debug_dummy_volume_ask += askVolume;
        }
    }
}