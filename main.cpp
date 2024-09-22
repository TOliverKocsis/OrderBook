#include <boost/lockfree/spsc_queue.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "dataset_process.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "order_utilities.hpp"

int main() {
    read_in_is_done = false;  // flag for consumer_thread to keep running
    std::thread producer_thread{LoadOrdersFromCSV};
    std::thread consumer_thread{ProcessOrderMessages};
    producer_thread.join();
    consumer_thread.join();

    std::cout << "Processing finished, trades recorded: " << order_book.GetTrades().size() << std::endl;
    std::cout << "Returned ask volume: " << debug_dummy_volume_ask << std::endl;
    std::cout << "Returned bid volume: " << debug_dummy_volume_bid << std::endl;

    return 0;
}
