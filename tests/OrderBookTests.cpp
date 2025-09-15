// tests/OrderBookTests.cpp

#include <gtest/gtest.h>
#include "../OrderBook.h" // Adjust path to your header
#include "../ThreadSafeQueue.h" // The test needs to provide a queue

// Test Suite: OrderBookTest
// Test Case: FullMatchExecution

TEST(OrderBookTest, FullMatchExecution) {
    // ARRANGE: Set up the initial state
    // We need a dummy queue for the OrderBook constructor, even if we don't check it here.
    // ThreadSafeQueue<ViewModel> ui_queue;
    OrderBook order_book; // The object we are testing

    // Add a resting SELL order to the book
    order_book.process_message(1, 101, 50, 15000, -1); // Type 1, ID 101, 50 shares @ $1.50, Sell

    // ACT: Trigger the event we want to test
    // Add an aggressive BUY order that should match the sell order
    order_book.process_message(1, 102, 50, 15000, 1); // Type 1, ID 102, 50 shares @ $1.50, Buy

    // ASSERT: Verify the outcome is correct
    // We can access private members because we declared this test as a 'friend'.

    // 1. Check that a trade was generated.
    ASSERT_EQ(order_book.trades_history.size(), 1);

    // 2. Check the details of the trade.
    const Trade& executed_trade = order_book.trades_history.front();
    EXPECT_EQ(executed_trade.buy_order_id, 102);
    EXPECT_EQ(executed_trade.sell_order_id, 101);
    EXPECT_EQ(executed_trade.price, 15000);
    EXPECT_EQ(executed_trade.quantity, 50);

    // 3. Check that the book is now empty since the orders were fully filled.
    ASSERT_TRUE(order_book.bids.empty());
    ASSERT_TRUE(order_book.asks.empty());
    ASSERT_TRUE(order_book.order_map.empty());
}