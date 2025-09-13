#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "Order.h"
#include "Trade.h"
#include "ViewModel.h"

#include <map>
#include <list>
#include <functional>

struct PriceLevelData {
    std::list<Order> orders;
    unsigned int total_quantity = 0;
    unsigned long order_count = 0;
};

class OrderBook {
public:
    void process_message(int event_type, long long order_id, unsigned int quantity, int price, int direction);
    void print_book();

    void set_current_timestamp(double time) {
        current_timestamp = time;
    }

    double get_current_timestamp() {
        return this->current_timestamp;
    }

    ViewModel get_view_model(int depth);

private:
    void handle_add(long long order_id, unsigned int quantity, int price, OrderSide side);
    void handle_cancel(long long order_id, unsigned int quantity);
    void handle_delete(long long order_id);
    void handle_execute(long long order_id, unsigned int quantity);

    std::map<int, PriceLevelData, std::greater<int>> bids;
    std::map<int, PriceLevelData> asks;
    std::unordered_map<long long, std::list<Order>::iterator> order_map;

    std::list<Trade> trades_history;
    double current_timestamp;
};

#endif //ORDER_BOOK>H