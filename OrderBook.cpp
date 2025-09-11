#include "OrderBook.h"
#include <iostream>

void OrderBook::process_message(int event_type, long long order_id, unsigned int quantity, int price, int direction) {
    OrderSide side = (direction == 1) ? OrderSide::BUY : OrderSide::SELL;
    switch(event_type) {
        case 1: //Add order
            handle_add(order_id, quantity, price, side);
            break;
        case 2: //Cancel order (update the order quantity)
            handle_cancel(order_id, quantity);
            break;
        case 3: //delete the order
            handle_delete(order_id);
            break;
        case 4: //execute an order
            handle_execute(order_id, quantity);
            break;
        default:
            break;
    }
}

void OrderBook::print_book() {
    std::cout << "BIDS:" << std::endl;
    for (const auto& level : bids) {
        for (const auto& order : level.second) {
            std::cout << " Price: " << level.first << ", Qty: " << order.quantity << ", ID: " << order.order_id << std::endl;
        }
    }

    std::cout << "ASKS:" << std::endl;
    for (const auto& level : asks) {
        for (const auto& order : level.second) {
            std::cout << " Price: " << level.first << ", Qty: " << order.quantity << ", ID: " << order.order_id << std::endl;
        }
    }
}


void OrderBook::handle_add(long long order_id, unsigned int quantity, int price, OrderSide side) {
    if (side == OrderSide::BUY) {
        bids[price].emplace_back(order_id, side, quantity, price);
        order_map[order_id] = std::prev(bids[price].end());
    } else {
        asks[price].emplace_back(order_id, side, quantity, price);
        order_map[order_id] = std::prev(asks[price].end());
    }
}


void OrderBook::handle_cancel(long long order_id, unsigned int quantity_to_cancel) {
    auto it = order_map.find(order_id);
    if (it == order_map.end()) {
        return;
    }
    
    auto list_iter = it->second;

    if (list_iter->quantity > quantity_to_cancel) {
        list_iter->quantity -= quantity_to_cancel;
    } else {
        handle_delete(order_id);
    }
}

void OrderBook::handle_delete(long long order_id) {
    auto it = order_map.find(order_id);
    if (it == order_map.end()) {
        return;
    }

    auto list_iter = it->second; // iterator to that specific order
    int price = list_iter->price; // get the price of the order
    OrderSide side = list_iter->side; // get the side of the order

    if (side == OrderSide::BUY) {
        auto& price_level_list = bids[price];
        price_level_list.erase(list_iter);
        if (price_level_list.empty()) {
            bids.erase(price);
        }
    } else {
        auto& price_level_list = asks[price];
        price_level_list.erase(list_iter);
        if(price_level_list.empty()) {
            asks.erase(price);
        }
    }

    order_map.erase(it);
}

void OrderBook::handle_execute(long long order_id, unsigned int executed_quantity) {
    handle_cancel(order_id, executed_quantity);
}