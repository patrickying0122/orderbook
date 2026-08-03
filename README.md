# orderbook

A limit order book with a matching engine, written in C++20 from scratch. Orders are added to the book, organized by side and price, and matched by price-time priority — the same rule real exchanges use. Prices are stored as integer ticks (`10050` = $100.50 at 1 tick = $0.01) rather than doubles, following real exchange feeds.

## What it does

- **Two-sided book.** Bids sort high-to-low, asks low-to-high, so the best price on each side is always the first entry. Orders at the same price queue in arrival order (time priority).
- **Matching on entry.** An incoming order walks the opposite side while it still crosses, filling level by level and generating a `Trade` for each fill. Whatever quantity survives rests in the book.
- **Price improvement.** Trades print at the *resting* order's price, not the incoming order's. A buy willing to pay 95 that hits an ask resting at 90 trades at 90 and keeps the difference.
- **Market orders.** `is_market` orders cross every level regardless of price. Any unfilled remainder is **cancelled, not rested** — a market order has no price, so there is no level it could rest at.
- **Cancellation.** `cancelOrder(id)` unlinks a resting order in constant time via an id → location index, and erases its price level if it was the last one there. Returns `false` for an id that is not resting — already filled, already cancelled, or never added.

Each side is a `std::map` of price → `std::list<Order>`: the map orders the levels, the list holds the queue at one level.

## Design note: one matching loop, two sides

Buy and sell matching are identical except for the direction of one comparison — and the two sides are different C++ types, since the bid map carries a `std::greater` comparator to sort high-to-low. Rather than duplicate the loop, `matchAgainst` is a template parameterised on both the level-map type and a `crosses` predicate supplied by the caller:

```cpp
matchAgainst(asks_, o, [&o](long lvl){ return o.is_market || o.price >= lvl; });  // buy
matchAgainst(bids_, o, [&o](long lvl){ return o.is_market || o.price <= lvl; });  // sell
```

The compiler stamps out two concrete functions — one per map type, each with its comparison baked in — so the choice costs nothing at runtime. No vtable, no indirect call, fully inlinable. `std::function` or a virtual method would express the same thing but resolve it while the program runs. Confirmed with `nm` on the test binary: two distinct `matchAgainst` symbols at two addresses.

Because a template must be instantiated where it is used, `matchAgainst`'s definition lives in [order_book.hpp](order_book.hpp) rather than the `.cpp` — a definition in the `.cpp` compiles to nothing and fails at link time.

## Design note: `list` per level, not `deque`

The queue at each price level started as a `std::deque` and was swapped to `std::list`. `deque` invalidates every iterator into it on `push_back`/`pop_front`; `std::list` iterators stay valid for the lifetime of the element they point at. That difference decides how cancels can work: with stable iterators the book can keep an id → iterator index and unlink a cancelled order in O(1), while with a `deque` those stored iterators would rot on the first fill.

The cost is real — `deque` stores orders in contiguous chunks that walk cache-friendly, `list` is a node per order and pointer chasing on every traversal. The trade is worth it because live order flow is mostly cancels rather than fills, so cancel cost dominates traversal cost.

Every method the swap touched — `push_back`, `front`, `pop_front`, `erase(it)`, `empty`, `size` — exists on both containers, so it was a one-line change to the two member declarations in [order_book.hpp](order_book.hpp#L51-L52) with no change to any method body.

## Design note: O(1) cancel via an id → location index

`cancelOrder` used to scan every level and every order looking for a matching id — O(L · N). It now does one hash lookup into an index that records where each resting order lives:

```cpp
struct Locator {
    bool is_buy{};                      // which map
    long price{};                       // which level
    std::list<Order>::iterator it;      // the exact node in that level's queue
};

std::unordered_map<int, Locator> index_;
```

The stored iterator is the point: `list::erase(it)` unlinks a known node by relinking its neighbours, no search involved.

The invariant is that `index_` mirrors the maps exactly — one entry per resting order, nothing else. Both directions need maintaining, and the easy half to forget is removal: when `matchAgainst` fully fills a resting order and pops it, the entry must go too, or a later cancel on that id erases a freed node. It also has to happen *before* the `pop_front`, since `resting` is a reference into the node being destroyed.

`find` is used rather than `operator[]` on both lookups — on a miss `operator[]` inserts, which would hand back a garbage iterator and quietly grow the index on every failed cancel.

**Complexity:** the hash lookup and the list unlink are O(1); finding the level by price to check whether it emptied is O(log L) in the number of price levels. Storing the map iterator instead of the price would flatten that, but the two sides are different types (the bid map carries `std::greater`), so one field cannot hold either without a `std::variant`.

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
| `test_cancel_basic` | level with two orders survives the first cancel, disappears on the second; unknown id returns `false` |
| `test_cancel_twice` | second cancel of the same id returns `false` instead of erasing a dead node |
| `test_cancel_after_full_fill` | filled order is gone from the index, so cancelling it returns `false` |
| `test_cancel_after_partial_fill` | partially filled order stays resting and stays cancellable |
| `test_cancel_market_and_relist` | market order is never indexed; a level emptied by cancel rebuilds cleanly |

## Known issues

- A resting remainder is copied into its level rather than moved.
- `Locator` stores the price rather than the map iterator, so the level lookup in `cancelOrder` is O(log L) rather than O(1).
- No test yet covers a market order that partially fills and has its remainder cancelled.
