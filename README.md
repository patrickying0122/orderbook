# orderbook

A limit order book written in C++20, built from scratch. Orders are added to the book, organized by side and price, and — as the project grows — matched by price-time priority, the same rule real exchanges use. Prices are stored as integer ticks (`10050` = $100.50 at 1 tick = $0.01) rather than doubles, following real exchange feeds.

## Build & run

```sh
clang++ -std=c++20 -Wall main.cpp -o ob && ./ob
```

Sample output:

```
[id 1] BUY 100 @ 10050
[id 2] BUY 200 @ 20050
[id 3] SELL 600 @ 40050
[id 4] SELL 400 @ 80050
```
