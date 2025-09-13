#ifndef TRADE_H
#define TRADE_H

struct Trade {
    long long buy_order_id;
    long long sell_order_id;
    unsigned int quantity;
    int price;
};

#endif // TRADE_H