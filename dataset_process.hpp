#ifndef DATASET_PROCESS_HPP
#define DATASET_PROCESS_HPP

#include <boost/lockfree/spsc_queue.hpp>
#include <string>
#include <atomic>
#include "order.hpp"
#include "order_book.hpp"



// Declare global variables using extern
extern boost::lockfree::spsc_queue<OrderMessage> order_messages;
extern OrderBook order_book;
extern std::atomic<bool> read_in_is_done;
extern uint32_t debug_dummy_volume_ask;
extern uint32_t debug_dummy_volume_bid;
extern std::string filename;

void ProcessOrderMessages();
void LoadOrdersFromCSV();



#endif //DATASET_PROCESS_HPP
