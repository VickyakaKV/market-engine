/*
* Script that lets users place orders and see the order book and trades executed.
*/

#include "order_book.hpp"
#include <iostream>
#include <string>
using namespace std;

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