#include "OrderBook.h"
#include "Trade.h"
#include "ViewModel.h"

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
        const auto& priceLevelData = level.second;
        for (const auto& order : priceLevelData.orders)
            std::cout << " Price: " << level.first << ", Qty: " << order.quantity << ", ID: " << order.order_id << std::endl;
    }

    std::cout << "ASKS:" << std::endl;
    for (const auto& level : asks) {
        const auto& priceLevelData = level.second;
        for (const auto& order : priceLevelData.orders)
            std::cout << " Price: " << level.first << ", Qty: " << order.quantity << ", ID: " << order.order_id << std::endl;
    }
}

//lobster data event type 4 are only for resting orders. it dont consider passive orders/market orders.
void OrderBook::handle_add(long long order_id, unsigned int quantity, int price, OrderSide side) {
    if (side == OrderSide::BUY) {
        bids[price].orders.emplace_back(order_id, side, quantity, price); // efficient than push back. no temoprary object created. it construct object in place.
        bids[price].order_count ++;
        bids[price].total_quantity += quantity;
        order_map[order_id] = std::prev(bids[price].orders.end());
    } else {
        asks[price].orders.emplace_back(order_id, side, quantity, price);
        asks[price].order_count ++;
        asks[price].total_quantity += quantity;
        order_map[order_id] = std::prev(asks[price].orders.end());
    }
}


void OrderBook::handle_cancel(long long order_id, unsigned int quantity_to_cancel) {
    auto it = order_map.find(order_id);
    if (it == order_map.end()) {
        return;
    }
    
    auto list_iter = it->second;

    if (list_iter->quantity <= quantity_to_cancel) {
        handle_delete(order_id);
    } else {
        list_iter->quantity -= quantity_to_cancel;
        //vto update cache
        int price = list_iter->price;
        OrderSide side = list_iter->side;

        if (side == OrderSide::BUY) {
            bids[price].total_quantity -= quantity_to_cancel;
        } else {
            asks[price].total_quantity -= quantity_to_cancel;
        }
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
    unsigned int quantity = list_iter-> quantity;

    if (side == OrderSide::BUY) {
        auto& price_level_data = bids[price];
        price_level_data.orders.erase(list_iter);
        price_level_data.order_count --;
        price_level_data.total_quantity -= quantity;
        if (price_level_data.orders.empty()) {
            bids.erase(price);
        }
    } else {
        auto& price_level_data = asks[price];
        price_level_data.orders.erase(list_iter);
        price_level_data.order_count --;
        price_level_data.total_quantity -= quantity;
        if(price_level_data.orders.empty()) {
            asks.erase(price);
        }
    }

    order_map.erase(it);
}

void OrderBook::handle_execute(long long order_id, unsigned int executed_quantity) {
    auto it = order_map.find(order_id);
    if (it == order_map.end()) {
        return;
    }

    auto iter_list = it->second;
    int price = iter_list->price;
    OrderSide side = iter_list->side;

    Trade new_trade;
    new_trade.quantity = executed_quantity;
    new_trade.price = price;

    if (side == OrderSide::BUY) {
        new_trade.buy_order_id = order_id;
        new_trade.sell_order_id = 0;
    } else {
        new_trade.buy_order_id = 0;
        new_trade.sell_order_id = order_id;
    }

    trades_history.push_back(new_trade);

    if (trades_history.size() > 20) { // only keeps lates 20 trades. 
        trades_history.pop_front();
    }

    handle_cancel(order_id, executed_quantity);
}

ViewModel OrderBook::get_view_model(int depth) {
    ViewModel vm;

    //populate bids
    auto bid_it = bids.begin();
    for (int i = 0; i < depth && bid_it != bids.end(); ++i, ++bid_it) {
        vm.bids.push_back(PriceLevel{bid_it->first, bid_it->second.total_quantity, bid_it->second.order_count});
    }

    //populate asks
    auto ask_it = asks.begin();
    for (int i = 0; i < depth && ask_it != asks.end(); ++i, ++ask_it) {
        vm.asks.push_back(PriceLevel{ask_it->first, ask_it->second.total_quantity, ask_it->second.order_count});
    }

    //populate recent trades in reverse for recent history
    int trade_count = 0;
    for (auto it = trades_history.rbegin(); it != trades_history.rend() && trade_count < 5; ++it, ++trade_count) {
        vm.recent_trades.push_back(*it);
    }

    //calculate mid price and spread
    if (!bids.empty() && !asks.empty()) {
        int best_bid = bids.begin()->first;
        int best_ask = asks.begin()->first;
        vm.mid_price = (best_bid + best_ask) / 2.0 / 10000.0;
        vm.spread = (best_ask - best_bid) / 10000.0;
    }

    //set the curent time

    auto now = get_current_timestamp();
    vm.current_timestamp = now;

    //formatted time for tui
    vm.formatted_time = vm.FormatTime(now);

    return vm;
}