#ifndef VIEW_MODEL_H
#define VIEW_MODEL_H
#include <sstream>
#include <iomanip>

#include <vector>
#include "Trade.h"

struct PriceLevel {
    int price;
    unsigned int total_quantity;
    unsigned long order_count;
};

struct ViewModel {
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
    std::vector<Trade> recent_trades;
    std::string ticker = "AAPL";
    double mid_price = 0.0;
    double spread = 0.0;
    double current_timestamp = 0.0;
    std::string formatted_time;
    
    static std::string FormatTime(double seconds) {
        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((seconds - hours * 3600) / 60);
        int secs = static_cast<int>(seconds) % 60;
        double fractional = seconds - static_cast<int>(seconds);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << hours << ":"
           << std::setw(2) << minutes << ":"
           << std::setw(2) << secs << "."
           << std::setw(6) << std::right << static_cast<int>(fractional * 1e6);
        return ss.str();
    }

};
#endif //VIEW_MODEL_H