#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <deque>
#include <optional>
#include <cassert>
#include "order_book.hpp"

int main(){
    OrderBook book{};

    // --- EDGE CASE 1: empty book ---
    // No orders added yet: every accessor should report "no value", and
    // printBook must not crash (it iterates, so an empty map => 0 rows).
    {
        OrderBook empty;
        assert(!empty.bestBid().has_value());
        assert(!empty.bestAsk().has_value());
        assert(!empty.spread().has_value());

    }
    {
    // --- EDGE CASE 2: one-sided book (bids only, asks still commented out) ---
    // Best bid exists (highest = 99); best ask and spread do not.
        OrderBook oneSided;
        Order o6 {6, true, 99, 4};
        oneSided.addOrder(o6);
        assert(oneSided.bestBid().has_value() && *oneSided.bestBid() == 99);
        assert(!oneSided.bestAsk().has_value());
        assert(!oneSided.spread().has_value());
    }
    // Insert prices deliberately OUT of order to prove the maps do the sorting.
    // Order{ id, is_buy, price, qty }.  is_buy=false -> ask (sell), true -> bid (buy).

    // addOrder takes Order& (it mutates o.qty while matching), so each order
    // needs to be a named variable -- a temporary can't bind to a non-const ref.
    // --- asks / sells ---
    Order a1 {1, false, 101, 10};   // ask @ 101
    Order a2 {2, false, 100, 5};    // ask @ 100  (best ask)
    Order a3 {3, false, 102, 7};    // ask @ 102
    Order a4 {4, false, 100, 3};    // ask @ 100 again -> level has 2 orders, qty 5+3=8
    book.addOrder(a1);
    book.addOrder(a2);
    book.addOrder(a3);
    book.addOrder(a4);
    // --- bids / buys ---
    // All below the best ask (100), so nothing crosses and everything rests.
    Order b5 {5, true, 98, 12};     // bid @ 98
    Order b6 {6, true, 99, 4};      // bid @ 99  (best bid)
    Order b7 {7, true, 97, 8};      // bid @ 97
    Order b8 {8, true, 99, 6};      // bid @ 99 again -> level has 2 orders, qty 4+6=10
    book.addOrder(b5);
    book.addOrder(b6);
    book.addOrder(b7);
    book.addOrder(b8);

    book.printBook();

    // The book may be one-sided here, so bestAsk()/spread() can be empty.
    // value_or(-1) prints -1 instead of dereferencing an empty optional (UB).
    std::cout << "\nbest ask = " << book.bestAsk().value_or(-1)
              << "   best bid = " << book.bestBid().value_or(-1)
              << "   spread = "   << book.spread().value_or(-1) << "\n";

    // --- O(1) cancel, via the id -> Locator index ---
    // Level 99 holds orders 6 and 8. Cancelling 6 leaves the level standing;
    // cancelling 8 empties it, so the level itself is erased and the best bid
    // drops to 98. Cancelling a dead id just reports false.
    assert(book.cancelOrder(6) == true);
    assert(book.bestBid().has_value() && *book.bestBid() == 99);   // order 8 still at 99
    assert(book.cancelOrder(8) == true);
    assert(book.bestBid().has_value() && *book.bestBid() == 98);   // level 99 erased
    assert(book.cancelOrder(6) == false);                          // already cancelled
    assert(book.cancelOrder(42) == false);                         // never existed

    std::cout << "\nAfter cancelling orders 6 and 8:\n";
    book.printBook();

    std::cout << "\nAll main() edge-case asserts passed.\n";
    return 0;
}




