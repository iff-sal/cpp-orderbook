#ifndef ORDER_H
#define ORDER_H

#include <cstdint>

enum class OrderSide {
    BUY,
    SELL
};

struct Order {
    long long order_id;
    OrderSide side;
    unsigned int quantity;
    int price;

    //constructure for the struct
    Order(long long orderId, OrderSide s, unsigned int q, int p) : order_id(orderId), side(s), quantity(q), price(p) {};
};

#endif //ORDER_H