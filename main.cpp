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
        oneSided.addOrder(Order{6,true,99,4});
        assert(oneSided.bestBid().has_value() && *oneSided.bestBid() == 99);
        assert(!oneSided.bestAsk().has_value());
        assert(!oneSided.spread().has_value());
    }
    // Insert prices deliberately OUT of order to prove the maps do the sorting.
    // Order{ id, is_buy, price, qty }.  is_buy=false -> ask (sell), true -> bid (buy).

    // --- asks / sells ---
    book.addOrder(Order{1, false, 101, 10});   // ask @ 101
    book.addOrder(Order{2, false, 100, 5});    // ask @ 100  (best ask)
    book.addOrder(Order{3, false, 102, 7});    // ask @ 102
    book.addOrder(Order{4, false, 100, 3});    // ask @ 100 again -> level has 2 orders, qty 5+3=8
    // --- bids / buys ---
    book.addOrder(Order{5, true, 98, 12});     // bid @ 98
    book.addOrder(Order{6, true, 99, 4});      // bid @ 99  (best bid)
    book.addOrder(Order{7, true, 97, 8});      // bid @ 97
    book.addOrder(Order{8, true, 99, 6});      // bid @ 99 again -> level has 2 orders, qty 4+6=10

    book.printBook();

    // The book may be one-sided here, so bestAsk()/spread() can be empty.
    // value_or(-1) prints -1 instead of dereferencing an empty optional (UB).
    std::cout << "\nbest ask = " << book.bestAsk().value_or(-1)
              << "   best bid = " << book.bestBid().value_or(-1)
              << "   spread = "   << book.spread().value_or(-1) << "\n";

    std::cout << "\nAll main() edge-case asserts passed.\n";
    return 0;
}




