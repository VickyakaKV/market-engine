# **Market Simulator**

This project contains a single script that lets users place orders for a single un-named stock and see the order book and trades executed in realtime.

Completely written in C++

The orders are matched in price-time-priority. 
Upto 3 decimal points are respected for prices - an implicit assumption that the un-named stock has a tick-size > 0.001.

### **Data structures used:**
- Max-heap (priority_queue) for frequent retreival of best prices and popping out  
  orders once they are executed completely. 
- Balanced-binary-tree (set) for book-keeping - to efficiently display unmatched
  orders, in price-priority (no time-priority since we aggregate orders across time, by price).

### **Explore and enhance:**

Research whether an ordered map instead of an ordered set can be more effective. 
The map can have independent Order objects that are keyed by price, quantities 
aggregated over orders submitted at the same price. This can make us do away 
with pointers, since there are no shared references between the queue and map.
The overhead will be more updates to Order objects in the map, now that they are
independent from the objects in priority queue. 

### **Supporting multiple stocks:**

Extend the current OrderBook class with a new StockOrderBook class to maintain
order books per stock - each stock with have their own StockOrderBook object to
add, execute and print orders/trades.
