#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "OrderBook.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path to the csv file>" << std::endl;
    }

    OrderBook order_book;
    std::string data_file_name = argv[1];
    std::ifstream message_file(data_file_name);

    if (!message_file.is_open()) {
        std::cerr << data_file_name << " coudnt be open!" << std::endl;
        return 1;
    }

    std::string line; //store each line reading

    while (std::getline(message_file, line)) {
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;
        while (std::getline(ss, item, ',')) {
            tokens.push_back(item);
        }

        if (tokens.size() != 6) {
            continue; // if the data is incomplete
        }

        double time = std::stod(tokens[0]);
        int event_type = std::stoi(tokens[1]);
        long long order_id = std::stoll(tokens[2]);
        unsigned int quantity = std::stoul(tokens[3]);
        int price = std::stoi(tokens[4]);
        int direction = std::stoi(tokens[5]);

        order_book.process_message(event_type, order_id, quantity, price, direction);
    }

    return 0;
}