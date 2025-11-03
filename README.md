# **Market Simulator**

This project contains a single script that lets users place orders for a single un-named stock and see the order book and trades executed in realtime.

Completely written in C++

The orders are matched in price-time-priority. 
Upto 3 decimal points are respected for prices - an implicit assumption that the un-named stock has a tick-size > 0.001.

### **Data structures used:**
- Two maps (balanced binary search trees):
  - One that maintains a map from price to a queue of unmatched orders at that price.
  - The other that maintains a map from the same set of prices as above, to the total volume of unmatched orders at that price 

An alternate implementation (in main.cpp) uses the following data structures
- Max-heap (priority_queue) for frequent retreival of best prices and popping out  
  orders once they are executed completely. 
- Balanced-binary-tree (set) for book-keeping - to efficiently display unmatched
  orders, in price-priority (no time-priority since we aggregate orders across time, by price).

### **Explore and enhance:**
- Research whether a segment tree can be effective when the price values are well bounded.
- A multi-map instead of two maps.

### **Supporting multiple stocks:**
Extend the current OrderBook class with a new StockOrderBook class to maintain
order books per stock - each stock with have their own StockOrderBook object to
add, execute and print orders/trades.

## Build Instructions
### Prerequisites
Make sure you have g++ installed and it supports C++20.

### Clone the repo
```
git clone https://github.com/VickyakaKV/market-engine.git
cd market-engine
```

### Compile and run
```
market-engine % g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude src/order_book.cpp app/market_engine.cpp -o market_engine
market-engine % ./market_engine
Enter trades in format <Side> <Quantity> <Price>
B 40 10


BUY            |           SELL
40@10          |               

B 10 10


BUY            |           SELL
50@10          |               

B 50 10.2345


BUY            |           SELL
50@10.234      |               
50@10          |               

S 50 11


BUY            |           SELL
50@10.234      |          50@11
50@10          |               

S 30 10.2222

30@10.234

BUY            |           SELL
20@10.234      |          50@11
50@10          |               

N 65 9.8
ERROR: Side should be either 'B' or 'S'
Ignoring input. Please re-enter:
S 65 9.8

20@10.234
40@10
5@10

BUY            |           SELL
5@10           |          50@11
```



