#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <mutex>

#include "OrderBook.h"
#include "ViewModel.h"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/table.hpp"


ftxui::Element CreatUI(const ViewModel& vm);

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_csv> <speed_factor>" << std::endl;
        std::cerr << "  speed_factor: 0 = max speed (backtest), 1 = real-time, 10 = 10x faster" << std::endl;
        return 1;
    }

    std::string data_file_name = argv[1];
    double speed_factor = 0.0;
    try {
        speed_factor = std::stod(argv[2]);
        if (speed_factor < 0) throw std::invalid_argument("Speed factor must be non-negative.");
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid speed factor provided. " << e.what() << std::endl;
        return 1;
    }

    // opening csv file
    std::ifstream message_file(data_file_name); //open input file stream
    if (!message_file.is_open()) {
        std::cerr << data_file_name << " coudnt be open!" << std::endl;
        return 1;
    }

    // ftxui setup
    auto&& screen = ftxui::ScreenInteractive::FitComponent();

    ViewModel shared_view_model;
    std::mutex view_model_mutex;
    bool simulation_running = true;

    
    //simulation thread - this thread read file and process message and update order book.
    //this run seperately from the ui rendering
    std::thread simulaiton_thread([&]() {
        //Orderbook is private member and other thread cant accees it
        OrderBook order_book;
        std::string line;
        //state variables for time tracking
        int line_count = 0;
        double last_timestamp = 0.0;
        bool is_first_line = true;

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

            //time siumlation logic
            if(is_first_line) {
                last_timestamp = time;
                is_first_line = false;
            } else {
                //a speed of 0 mean max speed
                if (speed_factor > 0) {
                    double time_delta_seconds = time - last_timestamp;
                    if (time_delta_seconds > 0) {
                        auto sleep_duration = std::chrono::duration<double>(time_delta_seconds / speed_factor);
                        std::this_thread::sleep_for(sleep_duration);
                    }
                }

                last_timestamp = time;
            }

            //process one message
            order_book.process_message(event_type, order_id, quantity, price, direction);
            order_book.set_current_timestamp(time);


            //ui update sampling - throttled to balance responsive with perfomance so simulation wont be slowed down
            line_count++;
            if (line_count % 20 == 0) { //rerender the ui only every x lines(line_count % 50 == 0). ftxui have some issue when rendering tables in high frequency. but matching engine will do its work as simulaiton time(ns).
                ViewModel snapshot = order_book.get_view_model(10); //get 10 price levels data

                {
                    std::lock_guard<std::mutex> lock(view_model_mutex);
                    shared_view_model = snapshot;
                }

                screen.PostEvent(ftxui::Event::Custom);
            }
        }

        simulation_running = false;
        screen.PostEvent(ftxui::Event::Custom); //final refresh
        screen.ExitLoopClosure()(); // ui loop stop
    });


    //ui rendering - the renderer capture viewmodel by reference (Main thread)
    auto renderer = ftxui::Renderer([&] {
        ViewModel local_vm_for_rendering;
        {
            std::lock_guard<std::mutex> lock(view_model_mutex);

            local_vm_for_rendering = shared_view_model;
        }
        return CreatUI(local_vm_for_rendering);

    });
    

    screen.Loop(renderer);

    simulaiton_thread.join();
    std::cout << "Simulaiton finished." << std::endl;

    return 0;
}

ftxui::Element CreatUI(const ViewModel& vm) {
    //bids table
    std::vector<std::vector<std::string>> bid_data;
    bid_data.push_back({" Count ", " Qty ", " Price "});
    for (const auto& level : vm.bids) {
        bid_data.push_back({
            " " + std::to_string(level.order_count) + " ",
            " " + std::to_string(level.total_quantity) + " ",
            " " + std::to_string(level.price / 10000.0) + " "
        });
    }
    auto bid_table = ftxui::Table(bid_data);
    bid_table.SelectAll().Border(ftxui::LIGHT);
    bid_table.SelectColumn(2).Decorate(ftxui::color(ftxui::Color::Green));
    bid_table.SelectAll().DecorateCells(ftxui::center); // Center all cells

    //asks table
    std::vector<std::vector<std::string>> ask_data;
    ask_data.push_back({" Price ", " Qty ", " Count "});
    for (const auto& level : vm.asks) {
        ask_data.push_back({
            " " + std::to_string(level.price / 10000.0) + " ",
            " " + std::to_string(level.total_quantity) + " ",
            " " + std::to_string(level.order_count) + " "
        });
    }
    auto ask_table = ftxui::Table(ask_data);
    ask_table.SelectAll().Border(ftxui::LIGHT);
    ask_table.SelectColumn(0).Decorate(ftxui::color(ftxui::Color::Red));
    ask_table.SelectAll().DecorateCells(ftxui::center);

    //trades table
    std::vector<std::vector<std::string>> trade_data;
    trade_data.push_back({" Price ", " Qty ", " Buy ID ", " Sell ID "});
    for (const auto& trade : vm.recent_trades) {
        trade_data.push_back({
            " " + std::to_string(trade.price / 10000.0) + " ",
            " " + std::to_string(trade.quantity) + " ",
            " " + std::to_string(trade.buy_order_id) + " ",
            " " + std::to_string(trade.sell_order_id) + " "
        });
    }
    auto trade_table = ftxui::Table(trade_data);
    trade_table.SelectAll().Border(ftxui::LIGHT);
    trade_table.SelectAll().DecorateCells(ftxui::center);

    //main layout
    auto order_book_layout = ftxui::hbox({
        bid_table.Render() | ftxui::flex,
        ask_table.Render() | ftxui::flex
    }) | ftxui::center; // Center the tables horizontally

    //header info
    std::stringstream ss_header;
    ss_header << "Ticker: " << vm.ticker << " | Sim Time: " << vm.formatted_time
              << " | Mid-Price: " << ((vm.mid_price > 0.0) ? std::to_string(vm.mid_price) : "N/A") << " | Spread: " << vm.spread;

    return ftxui::vbox({
        ftxui::text(ss_header.str()) | ftxui::bold | ftxui::center,
        ftxui::separator(),
        ftxui::text("ORDER BOOK") | ftxui::center,
        order_book_layout,
        ftxui::separator(),
        ftxui::text("RECENT TRADES") | ftxui::center,
        trade_table.Render() | ftxui::center
    }) | ftxui::border | ftxui::center; // Center the whole UI
}