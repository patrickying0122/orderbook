#include <iostream>
#include "order_book.hpp"

// Report EVERY failed check (unlike assert, which dies on the first).
// __LINE__ = the source line of the failing CHECK; #c = the condition as text.
#define CHECK(c) do { if(!(c)) { std::cout << "FAIL " << __LINE__ << ": " << #c << '\n'; ++failures; } } while(0)

// Test 1: a two-sided book reports the right best bid / best ask / spread.
void test_best_prices(int& failures){
    OrderBook b;
    // 4 asks (prices out of order on purpose; the map sorts them)
    b.addOrder(Order{1, false, 101, 10});
    b.addOrder(Order{2, false, 100, 5});
    b.addOrder(Order{3, false, 102, 7});
    b.addOrder(Order{4, false, 100, 3});   // 2nd order at 100
    // 4 bids
    b.addOrder(Order{5, true, 98, 12});
    b.addOrder(Order{6, true, 99, 4});
    b.addOrder(Order{7, true, 97, 8});
    b.addOrder(Order{8, true, 99, 6});      // 2nd order at 99

    CHECK(b.bestBid().has_value() && b.bestBid().value() == 99);   // highest bid
    CHECK(b.bestAsk().has_value() && b.bestAsk().value() == 100);  // lowest ask
    CHECK(b.spread().has_value()  && b.spread().value()  == 1);    // 100 - 99
}

// Test 2: cancel returns true, and a price level vanishes once its last order goes.
void test_cancel(int& failures){
    OrderBook b;
    b.addOrder(Order{1, true, 99, 5});   // the ONLY order at level 99
    b.addOrder(Order{2, true, 98, 3});   // level 98

    CHECK(b.bestBid().value() == 99);
    CHECK(b.cancelOrder(1) == true);                               // found & removed
    // Level 99 held one order; cancelling it should erase the whole level,
    // so the best bid drops to 98.
    CHECK(b.bestBid().has_value() && b.bestBid().value() == 98);
    CHECK(b.cancelOrder(999) == false);                            // no such id
}

// Test 3: an empty book has no best bid/ask/spread, and printing it must not crash.
void test_empty(int& failures){
    OrderBook b;
    CHECK(!b.bestBid().has_value());
    CHECK(!b.bestAsk().has_value());
    CHECK(!b.spread().has_value());
}

int main(){
    int failures = 0;

    test_best_prices(failures);
    test_cancel(failures);
    test_empty(failures);

    if(failures == 0) std::cout << "\nALL TESTS PASSED\n";
    else              std::cout << '\n' << failures << " CHECK(S) FAILED\n";

    return failures == 0 ? 0 : 1;   // non-zero exit signals failure to make/CI
}
