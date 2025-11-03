#include <iostream>
#include <map>
#include <queue>
#include <string>
using namespace std;

// Used to display Order book columns - BUY and SELL
const size_t COLUMN_WIDTH      = 15; 
const string ORDER_BOOK_HEADER = "BUY            |           SELL"; 

 // We scale price by this factor and remove the decimal part, which is essentially 
 // enforcing a tick size of 0.001
const long SCALE_FACTOR = 1000;

// A basic structure of an order. Note that the price here is scaled and hence in "long" type
struct Order {
    char side;
    long quantity;
    long price;
    int timestamp;

    Order (char s, long q, long p, int t) : side(s), quantity(q), price(p), timestamp(t) {} 
};

class OrderBook {
    /*
    * Maintains an Exchange Order Book and provides the following functionalities:
    * - Add a new order (side, quantity, price, timestamp)
    * - Execute trades by matching the orders (and also print them)
    * - Print Order Book status
    *
    * Data structure used:
    *  - An ordered map (a balanced binary tree) from scaled price to a queue of 
    *    unmatched orders submitted at that price.
    *  - Another ordered map with the same keys as above, the values being the 
    *    total volume of unmatched orders at that price.
    *
    * TODO: 
    * We could use a single multi-map instead of two maps with same keys at all times. 
    * I've used two just for simplicity.
    */

private:
    // Orders are maintained in a queue, keyed by price in an ordered map.
    map<long, deque<Order>> buy_orders;
    map<long, deque<Order>> sell_orders;

    // Total volume at each price is stored in ordered maps.
    map<long, long long> total_buy_orders_at_price;
    map<long, long long> total_sell_orders_at_price;

public:
    /*
    * @brief
    * Create a new Order (buy or sell based on "side") object with the given price,
    * quantity and timestamp. Timestamp is used to determine the price at which a
    * trade should be executed, based on whether the buy or sell order came first.
    *
    * @param side:         'B' for buy or 'S for sell.
    * @param quantity_str: Order quantity as a string.
    * @param price_str:    Order price as a string.
    * @param price_str:    Order price as a string.
    * @param timestamp:    Timestamp associated with the order.
    *
    * @return: true if the order was added to the book successfully, false otherwise.
    */
    bool add_order(char side, string quantity_str, string price_str, int timestamp);

    /*
    * Match the highest bid with the least ask and print the corresponding trades in sequence.
    */
    void execute_and_print_trades();

    /* 
    * Print the current state of the order book i.e only the unmatched orders.
    * Orders are group by price (using the map) and we display them in price-priority: highest bid and lowest ask first.
    */
    void print_order_book();
};