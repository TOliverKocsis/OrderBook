#include <fstream>
#include <iostream>
#include "Order.hpp"
#include "OrderBook.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>


OrderType stringToOrderType(const std::string& str) {
    if (str == "buy") return OrderType::buy;
    if (str == "sell") return OrderType::sell;
    return OrderType::undefined;
}

/*
 * Load orders from a csv file if the file exists/
 * csv file must be in the OrderType,OrderId,Price,Quantity
 * order. Copy all to a vector of Order type, and return it.
 */
std::vector<Order> loadOrdersFromCSV(const std::string& filename) {
    std::vector<Order> orders;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string line;
        // skip the header
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::string word;
            std::stringstream ss(line);
            Order order;
            std::string orderTypeStr;

            std::getline(ss, orderTypeStr, ',');
            order.ordertype = stringToOrderType(orderTypeStr);

            std::getline(ss, word, ',');
            order.orderId = std::stoi(word);

            std::getline(ss, word, ',');
            order.price = std::stoi(word);

            std::getline(ss, word, ',');
            order.quantity = std::stoi(word);

            orders.push_back(order);
        }
        file.close();
    }

    return orders;
}

/*void saveTradesToCSV(const std::string& filename, const std::vector<Trade>& trades) {
    std::ofstream file(filename);

    if (file.is_open()) {
        //add header
        file << "BuyOrderId,SellOrderId,Price,Quantity\n";
        for (const auto& trade : trades) {
            file << trade.buyOrderId << ',' << trade.sellOrderId << ',' << trade.price << ',' << trade.quantity << '\n';
        }
        file.close();
    }
}*/

int main() {
    //using main as the a test function for now

    //acceptance test
    //load orders from csv file
    std::vector<Order> orders = loadOrdersFromCSV("../OrderExamples/AcceptanceOrders.csv");

    OrderBook orderBook;

    for(const Order acceptanceOrder: orders) {
        orderBook.AddOrder(acceptanceOrder);
    }


    // Perform operations on the order book
    // Order Ids uniq and autoincremented during a day
    Order buyorder1{OrderType::buy, 1, 100, 10};
    Order sellorder1{OrderType::sell, 2, 105, 10};
    orderBook.AddOrder(buyorder1);
    orderBook.AddOrder(sellorder1);

    return 0;
}
