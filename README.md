# orderbook

A limit order book with a matching engine, written in C++20 from scratch. Orders are added to the book, organized by side and price, and matched by price-time priority — the same rule real exchanges use. Prices are stored as integer ticks (`10050` = $100.50 at 1 tick = $0.01) rather than doubles, following real exchange feeds.

## What it does

- **Two-sided book.** Bids sort high-to-low, asks low-to-high, so the best price on each side is always the first entry. Orders at the same price queue in arrival order (time priority).
- **Matching on entry.** An incoming order walks the opposite side while it still crosses, filling level by level and generating a `Trade` for each fill. Whatever quantity survives rests in the book.
- **Price improvement.** Trades print at the *resting* order's price, not the incoming order's. A buy willing to pay 95 that hits an ask resting at 90 trades at 90 and keeps the difference.
- **Market orders.** `is_market` orders cross every level regardless of price. Any unfilled remainder is **cancelled, not rested** — a market order has no price, so there is no level it could rest at.
- **Cancellation.** `cancelOrder(id)` removes a resting order and erases its price level if it was the last one there.

## Design note: one matching loop, two sides

Buy and sell matching are identical except for the direction of one comparison — and the two sides are different C++ types, since the bid map carries a `std::greater` comparator to sort high-to-low. Rather than duplicate the loop, `matchAgainst` is a template parameterised on both the level-map type and a `crosses` predicate supplied by the caller:

```cpp
matchAgainst(asks_, o, [&o](long lvl){ return o.is_market || o.price >= lvl; });  // buy
matchAgainst(bids_, o, [&o](long lvl){ return o.is_market || o.price <= lvl; });  // sell
```

The compiler stamps out two concrete functions — one per map type, each with its comparison baked in — so the choice costs nothing at runtime. No vtable, no indirect call, fully inlinable. `std::function` or a virtual method would express the same thing but resolve it while the program runs. Confirmed with `nm` on the test binary: two distinct `matchAgainst` symbols at two addresses.

Because a template must be instantiated where it is used, `matchAgainst`'s definition lives in [order_book.hpp](order_book.hpp) rather than the `.cpp` — a definition in the `.cpp` compiles to nothing and fails at link time.

## Build & run

```sh
make test && ./test    # unit tests
make ob   && ./ob      # demo program
```

Sample book:

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
```

Each row is one price level: `price | total quantity across all orders at that price | number of orders resting there`. Sells are listed high-to-low and buys high-to-low, so the **best ask (100)** and **best bid (99)** sit next to each other in the middle — the gap between them is the **spread (1)**. The `100` sell level shows `8 | 2`: two separate orders (qty 5 and 3) resting at the same price, aggregated.

## Tests

[tests.cpp](tests.cpp) reports every failing check rather than dying on the first, and exits non-zero so `make` and CI can see failures. Current coverage:

| test | scenario |
| --- | --- |
| `test_buy_sweeps_two_ask_levels` | buy sweeps one ask level whole, partially fills the next |
| `test_sell_sweeps_two_bid_levels` | mirror on the sell side; checks `buy_id`/`sell_id` land in the right fields when the incoming order is the seller |
| `test_market_buy` | market buy with a junk price field still sweeps both levels |

## Known issues

- `main.cpp` passes `Order` temporaries to `addOrder`, which now takes `Order&` — the `ob` target does not currently build. `make test` is unaffected.
- No test yet covers a market order whose remainder must be cancelled (the current one fills completely).
- `cancelOrder` has no test coverage.
