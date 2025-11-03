/*
* Script that lets users place orders and see the order book and trades executed.
*/

#include <iostream>
#include <algorithm>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <vector>
using namespace std;

size_t COLUMN_WIDTH      = 15; // Used to display Order book columns - BUY and SELL
string ORDER_BOOK_HEADER = "BUY            |           SELL"; 
float  PRECISION         = 0.001f; // Ignoring anything after 3 decimal places in stock price


struct Order {
    char side;
    int quantity;
    float price;
    int timestamp;
};

// Higher bid followed by earlier bid => more priority
struct CompareBuyOrders {
    bool operator()(const Order* a, const Order* b) const {
        if (abs(a->price - b->price) < PRECISION) 
            return a->timestamp > b->timestamp;
        return a->price < b->price;
    }
};

// Lower ask followed by earlier ask => more priority
struct CompareSellOrders {
    bool operator()(const Order* a, const Order* b) const {
        if (abs(a->price - b->price) < PRECISION) 
            return a->timestamp > b->timestamp;
        return a->price > b->price;
    }
};

class OrderBook {
    /*
    * Maintains an Exchange Order Book and provides the following functionalities:
    * - Add a new order (side, quantity, price, timestamp)
    * - Execute trades by matching the orders (and also print them)
    * - Print Order Book status
    *
    * Data structures used:
    * - Max-heap (priority_queue) for frequent retreival of best prices and popping out  
    *   orders once they are executed completely. 
    * - Balanced-binary-tree (set) for book-keeping - to efficiently display unmatched
    *   orders, in price-priority (no time-priority since we aggregate orders across time, by price).
    *
    * TODO: 
    * Explore whether an ordered map instead of an ordered set can be more effective. 
    * The map can have independent Order objects that are keyed by price, quantities 
    * aggregated over orders submitted at the same price. This can make us do away 
    * with pointers, since there are no shared references between the queue and map.
    * The overhead will be more updates to Order objects in the map, now that they are
    * independent from the objects in priority queue. 
    */

private:
    // Orders are maintained in a max-heap in price-time-priority - Bids and Asks separately.
    priority_queue<Order*, vector<Order*>, CompareBuyOrders>  buy_order_queue;
    priority_queue<Order*, vector<Order*>, CompareSellOrders> sell_order_queue;

    // Orders are also maintained in a balanced-binary-tree for book-keeping - Bids and Asks separately.
    set<Order*, CompareBuyOrders>  buy_order_set;
    set<Order*, CompareSellOrders> sell_order_set;

    template <class Itr>
    pair<string, Itr> get_next_entry(Itr it, Itr it_end, bool align) {
        /*
        * Utility function used to get the total quantity of order for a given price.
        * The "given price" part is inferred from the corresponding order book's 
        * iterator that is passed. We increment the iterator till we see all 
        * unmatched orders in the given price, and aggregate the quantities.  
        */

        int cum_order_size     = (*it)->quantity;
        float last_order_price = (*it)->price;
        while (++it != it_end && (*it)->price == last_order_price)
            cum_order_size += (*it)->quantity;
        
        string cell = format("{}@{}", cum_order_size, last_order_price);
        size_t pad  = COLUMN_WIDTH - cell.size();
        align ? cell.insert(0, pad, ' ') : cell.append(pad, ' ');

        return {cell, it};
    }

    enum class ValidationResult {
        VALID,
        INVALID_SIDE,
        INVALID_QUANTITY,
        INVALID_PRICE
    };

    ValidationResult validate_inputs(char side, string quantity, string price) {
        if (side != 'B' && side != 'S')
            return ValidationResult::INVALID_SIDE;
        if (!regex_match(quantity, std::regex("^[1-9][0-9]*$")))
            return ValidationResult::INVALID_QUANTITY;
        if (!regex_match(price, std::regex("^[0-9]*\\.?[0-9]+$")) || stof(price) < PRECISION)
            return ValidationResult::INVALID_PRICE;
        return ValidationResult::VALID;
    }

    string input_validation_message(ValidationResult result) {
        switch (result) {
            case ValidationResult::VALID:
                return "Good";
            case ValidationResult::INVALID_SIDE:
                return "Side should be either \'B\' or \'S\'";
            case ValidationResult::INVALID_QUANTITY:
                return "Order quantity should be a positive integer";
            case ValidationResult::INVALID_PRICE:
                return format("Price should be a positive value >= tick size ({:.3f})", PRECISION);
        }
        return "Unknown validation result";
    }

public:
    bool add_order(char side, string quantity_str, string price_str, int timestamp) {
        /*
        * Create a new Order (buy or sell based on "side") object with the given
        * price, quantity and timestamp. Timestamp is used to to determine 
        * price-time-priority when matching buy and sell orders.
        * It is also used to determine the price at which a trade should be executed, 
        * based on whether the buy or sell order came first.
        */

        // Validate inputs and create a new Order.
        ValidationResult validation_result = validate_inputs(side, quantity_str, price_str);
        if (validation_result != ValidationResult::VALID) {
            cout << "ERROR: " << input_validation_message(validation_result) << endl;
            return false;
        }

        // Truncate price to 3 decimal places (PRECISION sets this behaviour)
        float price      = stof(price_str);
        int scale_factor = ceil(1 / PRECISION);
        long scaled      = static_cast<long> (price * scale_factor);
        price            = static_cast<double>(scaled) / scale_factor;

        int quantity = stoi(quantity_str);
        Order* order = new Order{side, quantity, price, timestamp};

        // Add new order to the appropriate order queue and set 
        side == 'B' ? buy_order_queue.push(order) : sell_order_queue.push(order);
        side == 'B' ? buy_order_set.insert(order) : sell_order_set.insert(order);

        return true;
    }

    void execute_and_print_trades() {
        /*
        * Start by matching the most enticing buy order with the most enticing sell order.
        * Keep doing the above until the maximum bid is less than the minimum ask. 
        */

        while(!(buy_order_queue.empty() || sell_order_queue.empty())) {
            Order* best_buy_order  = buy_order_queue.top();
            Order* best_sell_order = sell_order_queue.top();
            if (best_buy_order->price < best_sell_order->price) break;

            int   trade_quantity = min(best_buy_order->quantity, best_sell_order->quantity);
            float trade_price    = best_buy_order->timestamp > best_sell_order->timestamp ? 
                                   best_sell_order->price : best_buy_order->price;

            // Print the trade that is to be executed                 
            cout << trade_quantity << "@" << trade_price << endl;

            // Update order quantities as per executed trade
            best_buy_order->quantity  -= trade_quantity;
            best_sell_order->quantity -= trade_quantity;

            // Remove orders that are completely matched
            auto remove_empty_order = [&](auto*& order, auto& order_queue, auto& order_set) {
                order_queue.pop();
                order_set.erase(order);
                delete order;
                order = order_queue.top();
            };

            if (best_buy_order->quantity == 0)  remove_empty_order(best_buy_order, buy_order_queue, buy_order_set);
            if (best_sell_order->quantity == 0) remove_empty_order(best_sell_order, sell_order_queue, sell_order_set);
        }
    }

    void print_order_book() {
        /* 
        * Print the current state of the order book i.e only the unmatched orders.
        * We group orders by price and display them in price-priority: highest bid and lowest ask.
        */

        cout << ORDER_BOOK_HEADER << endl;
        
        auto itB = buy_order_set.rbegin(),  itBend = buy_order_set.rend();
        auto itS = sell_order_set.rbegin(), itSend = sell_order_set.rend();

        while (itB != itBend || itS != itSend) {
            string row = "";

            auto append_cell = [&](auto& it, auto end, bool align) {
                if (it == end) return string(COLUMN_WIDTH, ' ');
                auto [cell, next] = get_next_entry(it, end, align);
                it = next;
                return cell;
            };
            
            row += append_cell(itB, itBend, 0);
            row += '|';
            row += append_cell(itS, itSend, 1);
            cout << row << endl;
        }
    }
};


int main() {
    cout << "Enter trades in format <Side> <Quantity> <Price>" << endl;
    char side;
    string quantity, price;
    // We could have the actual UTC seconds since epoch value here. 
    // For now, using just a counter for simplicity.
    int timestamp = 0;  

    OrderBook order_book;
    while (cin >> side >> quantity >> price) {
        bool success = order_book.add_order(side, quantity, price, ++timestamp);
        if (!success) {
            cout << "Ignoring input. Please re-enter:" << endl;
            continue;
        }
        order_book.execute_and_print_trades();
        order_book.print_order_book();
    }
}