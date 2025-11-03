#include "order_book.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <string>
using namespace std;


enum class ValidationResult {
    /* The four horsemen of invalid input */
    VALID,
    INVALID_SIDE,
    INVALID_QUANTITY,
    INVALID_PRICE
};

ValidationResult validate_inputs(char side, string quantity, string price) {
    /*
    Returns the result of input validation.
    If there are several invalidities in the input, the first of them is reported.
    */
    if (side != 'B' && side != 'S')
        return ValidationResult::INVALID_SIDE;
    if (!regex_match(quantity, std::regex("^[1-9][0-9]*$")))
        return ValidationResult::INVALID_QUANTITY;
    if (!regex_match(price, std::regex("^[0-9]*\\.?[0-9]+$")) || stof(price) < 1.0/SCALE_FACTOR)
        return ValidationResult::INVALID_PRICE;
    return ValidationResult::VALID;
}

string input_validation_message(ValidationResult result) {
    /*
    Returns a human-readable message corresponding to the input validation result.
    */
    switch (result) {
        case ValidationResult::VALID:
            return "Good";
        case ValidationResult::INVALID_SIDE:
            return "Side should be either \'B\' or \'S\'";
        case ValidationResult::INVALID_QUANTITY:
            return "Order quantity should be a positive integer";
        case ValidationResult::INVALID_PRICE:
            return format("Price should be a positive value >= tick size ({:.3f})", 1.0/SCALE_FACTOR);
    }
    return "Unknown validation result";
}


bool OrderBook::add_order(char side, string quantity_str, string price_str, int timestamp) {

    // Validate inputs and create a new Order.
    ValidationResult validation_result = validate_inputs(side, quantity_str, price_str);
    if (validation_result != ValidationResult::VALID) {
        cout << "ERROR: " << input_validation_message(validation_result) << endl;
        return false;
    }

    // Scale price by SCALE_FACTOR (inverse of tick size = 1/0.001 = 1000) and ignore the decimal part.
    long price    = static_cast<long> (stof(price_str) * SCALE_FACTOR);
    long quantity = stol(quantity_str);
    
    // Add new order to the appropriate order queue and set 
    side == 'B' ? buy_orders[price].emplace_back(side, quantity, price, timestamp) 
                : sell_orders[price].emplace_back(side, quantity, price, timestamp);  
    
    side == 'B' ? total_buy_orders_at_price[price] += quantity 
                : total_sell_orders_at_price[price] += quantity;  
                  
    return true;
}

void OrderBook::execute_and_print_trades() {

    // Start by matching the most enticing buy order with the most enticing sell order.
    // Keep going until the maximum bid is less than the minimum ask. 
    while(!(buy_orders.empty() || sell_orders.empty())) {
        Order best_buy_order  = buy_orders.rbegin()->second.front();
        Order best_sell_order = sell_orders.begin()->second.front();

        long buy_order_price  = best_buy_order.price;
        long sell_order_price = best_sell_order.price;

        if (buy_order_price < sell_order_price) break;

        long trade_quantity = min(best_buy_order.quantity, best_sell_order.quantity);
        long trade_price    = best_buy_order.timestamp > best_sell_order.timestamp ? 
                              sell_order_price : buy_order_price;

        // Print the trade that is to be executed                 
        cout << "\n" << trade_quantity << "@" << (trade_price * 1.0f / SCALE_FACTOR);

        // Update order quantities as per executed trade
        best_buy_order.quantity  -= trade_quantity;
        best_sell_order.quantity -= trade_quantity;

        buy_orders.rbegin()->second.front().quantity -= trade_quantity;
        sell_orders.begin()->second.front().quantity -= trade_quantity;

        total_buy_orders_at_price[buy_order_price]  -= trade_quantity;
        total_sell_orders_at_price[sell_order_price] -= trade_quantity;

        if (best_buy_order.quantity == 0) {
            buy_orders.rbegin()->second.pop_front();
            if (total_buy_orders_at_price[buy_order_price] == 0) {
                assert(buy_orders[buy_order_price].empty());
                total_buy_orders_at_price.erase(buy_order_price);
                buy_orders.erase(buy_order_price);
            }
        }
        if (best_sell_order.quantity == 0) {
            sell_orders.begin()->second.pop_front();
            if (total_sell_orders_at_price[sell_order_price] == 0) {
                assert(sell_orders[sell_order_price].empty());
                total_sell_orders_at_price.erase(sell_order_price);
                sell_orders.erase(sell_order_price);
            }
        }
    }
    cout << endl;
}

void OrderBook::print_order_book() {

    // Start by printing the order book header
    cout << "\n" << ORDER_BOOK_HEADER;
    
    // Display the buy orders from the right end i.e maximum price first. For sell orders, minimum price first.
    auto itB = total_buy_orders_at_price.rbegin(), itBend = total_buy_orders_at_price.rend();
    auto itS = total_sell_orders_at_price.begin(), itSend = total_sell_orders_at_price.end();

    while (itB != itBend || itS != itSend) {
        string row = "";

        auto append_cell = [&](auto& it, auto end, bool align) {
            if (it == end) return string(COLUMN_WIDTH, ' '); // Empty cell when no orders left.
            string cell = format("{}@{}", it->second, it->first * 1.0f / SCALE_FACTOR);

            // Get the right padding for the cell's contents so all rows are aligned.
            size_t pad  = COLUMN_WIDTH - cell.size();
            align ? cell.insert(0, pad, ' ') : cell.append(pad, ' ');
            it++;
            return cell;
        };
        
        row += append_cell(itB, itBend, 0);
        row += '|';
        row += append_cell(itS, itSend, 1);
        cout << "\n" << row;
    }
    cout << endl;
}
