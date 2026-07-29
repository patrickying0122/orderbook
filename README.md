# orderbook

A limit order book written in C++20, built from scratch. Orders are added to the book, organized by side and price, and — as the project grows — matched by price-time priority, the same rule real exchanges use. Prices are stored as integer ticks (`10050` = $100.50 at 1 tick = $0.01) rather than doubles, following real exchange feeds.

## Build & run

```sh
clang++ -std=c++20 -Wall main.cpp -o ob && ./ob
```

Sample output:

```
price|total qty|#orders
--- SELL ---
102 | 7 | 1
101 | 10 | 1
100 | 8 | 2
--- BUY ---
99 | 10 | 2
98 | 12 | 1
97 | 8 | 1

best ask = 100   best bid = 99   spread = 1

All main() edge-case asserts passed.
```

Each row is one price level: `price | total quantity across all orders at that price | number of orders resting there`. Sells are listed high-to-low and buys high-to-low, so the **best ask (100)** and **best bid (99)** sit next to each other in the middle — the gap between them is the **spread (1)**. Note the `100` sell level shows `8 | 2`: two separate orders (qty 5 and 3) resting at the same price, aggregated.
