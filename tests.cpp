#include <iostream>
#include "order_book.hpp"

// Report EVERY failed check (unlike assert, which dies on the first).
// __LINE__ = the source line of the failing CHECK; #c = the condition as text.
#define CHECK(c) do { if(!(c)) { std::cout << "FAIL " << __LINE__ << ": " << #c << '\n'; ++failures; } } while(0)

void test_no_crossing(int& failures){
    OrderBook book;
    Order a {1,false,100,100};
    Order b {2, true,99,100};
    book.addOrder(a);
    book.addOrder(b);
    CHECK(book.tradeAccessor().size()==0);
}

void test_one_trade(int& failures){
    OrderBook book;
    Order a {1,false,100,100};
    Order b {2, true,100,100};
    book.addOrder(a);
    book.addOrder(b);
    auto trades = book.tradeAccessor();
    CHECK(trades.size() == 1);
    if (trades.size()==1){
        CHECK(trades[0].buy_id == 2);
        CHECK(trades[0].sell_id == 1);
        CHECK(trades[0].price == 100);
        CHECK(trades[0].qty == 100);
    }
}

// Incoming buy has MORE qty than the resting ask: the ask gets fully eaten
// (1 trade, ask level erased) and the leftover qty rests as a new bid.
void test_incoming_bigger(int& failures){
    OrderBook book;
    Order a {1, false, 100, 50};   // resting ask: 50 @ 100
    Order b {2, true,  100, 100};  // incoming buy: 100 @ 100 (bigger)
    book.addOrder(a);
    book.addOrder(b);

    auto trades = book.tradeAccessor();
    CHECK(trades.size() == 1);
    if (trades.size() == 1) {
        CHECK(trades[0].buy_id  == 2);
        CHECK(trades[0].sell_id == 1);
        CHECK(trades[0].price   == 100);
        CHECK(trades[0].qty     == 50);      // fully consumes the resting ask
    }

    CHECK(!book.bestAsk().has_value());        // ask level fully consumed, erased
    CHECK(book.bestBid().has_value() && book.bestBid().value() == 100); // remainder rests
}

// Resting ask has MORE qty than the incoming buy: the buy is fully filled
// (1 trade), the ask just shrinks, and nothing new rests as a bid.
void test_resting_bigger(int& failures){
    OrderBook book;
    Order a {1, false, 100, 100};  // resting ask: 100 @ 100 (bigger)
    Order b {2, true,  100, 40};   // incoming buy: 40 @ 100 (smaller)
    book.addOrder(a);
    book.addOrder(b);

    auto trades = book.tradeAccessor();
    CHECK(trades.size() == 1);
    if (trades.size() == 1) {
        CHECK(trades[0].buy_id  == 2);
        CHECK(trades[0].sell_id == 1);
        CHECK(trades[0].price   == 100);
        CHECK(trades[0].qty     == 40);      // incoming fully filled
    }

    CHECK(book.bestAsk().has_value() && book.bestAsk().value() == 100); // resting order still there, just smaller
    CHECK(!book.bestBid().has_value());        // nothing left to rest
}

// Two ask levels; incoming buy fully sweeps the cheaper level and partially
// fills the next one. Exercises multi-level walking AND price improvement
// (trades must print at each resting level's own price, not the buy's price).
void test_sweep_two_levels(int& failures){
    OrderBook book;
    Order s1 {1, false, 90, 100};   // ask level 1: 100 @ 90
    Order s2 {2, false, 95, 50};    // ask level 2: 50 @ 95
    Order b  {3, true,  95, 130};   // incoming buy: 130 @ 95
    book.addOrder(s1);
    book.addOrder(s2);
    book.addOrder(b);

    auto trades = book.tradeAccessor();
    CHECK(trades.size() == 2);
    if (trades.size() == 2) {
        CHECK(trades[0].buy_id  == 3);
        CHECK(trades[0].sell_id == 1);
        CHECK(trades[0].price   == 90);    // resting level's price, not the buy's 95
        CHECK(trades[0].qty     == 100);   // level 1 fully swept

        CHECK(trades[1].buy_id  == 3);
        CHECK(trades[1].sell_id == 2);
        CHECK(trades[1].price   == 95);
        CHECK(trades[1].qty     == 30);    // 130 - 100 leftover eats only part of level 2
    }

    CHECK(book.bestAsk().has_value() && book.bestAsk().value() == 95); // level 90 fully erased
    CHECK(!book.bestBid().has_value());     // 100 + 30 = 130, buy fully consumed
}


// // Test 1: a two-sided book reports the right best bid / best ask / spread.
// void test_best_prices(int& failures){
//     OrderBook b;
//     // 4 asks (prices out of order on purpose; the map sorts them)
//     b.addOrder(Order{1, false, 101, 10});
//     b.addOrder(Order{2, false, 100, 5});
//     b.addOrder(Order{3, false, 102, 7});
//     b.addOrder(Order{4, false, 100, 3});   // 2nd order at 100
//     // 4 bids
//     b.addOrder(Order{5, true, 98, 12});
//     b.addOrder(Order{6, true, 99, 4});
//     b.addOrder(Order{7, true, 97, 8});
//     b.addOrder(Order{8, true, 99, 6});      // 2nd order at 99

//     CHECK(b.bestBid().has_value() && b.bestBid().value() == 99);   // highest bid
//     CHECK(b.bestAsk().has_value() && b.bestAsk().value() == 100);  // lowest ask
//     CHECK(b.spread().has_value()  && b.spread().value()  == 1);    // 100 - 99
// }

// // Test 2: cancel returns true, and a price level vanishes once its last order goes.
// void test_cancel(int& failures){
//     OrderBook b;
//     b.addOrder(Order{1, true, 99, 5});   // the ONLY order at level 99
//     b.addOrder(Order{2, true, 98, 3});   // level 98

//     CHECK(b.bestBid().value() == 99);
//     CHECK(b.cancelOrder(1) == true);                               // found & removed
//     // Level 99 held one order; cancelling it should erase the whole level,
//     // so the best bid drops to 98.
//     CHECK(b.bestBid().has_value() && b.bestBid().value() == 98);
//     CHECK(b.cancelOrder(999) == false);                            // no such id
// }

// // Test 3: an empty book has no best bid/ask/spread, and printing it must not crash.
// void test_empty(int& failures){
//     OrderBook b;
//     CHECK(!b.bestBid().has_value());
//     CHECK(!b.bestAsk().has_value());
//     CHECK(!b.spread().has_value());
// }

int main(){
    int failures = 0;

    // test_best_prices(failures);
    // test_cancel(failures);
    // test_empty(failures);
    test_no_crossing(failures);
    test_one_trade(failures);
    test_incoming_bigger(failures);
    test_resting_bigger(failures);
    test_sweep_two_levels(failures);

    if(failures == 0) std::cout << "\nALL TESTS PASSED\n";
    else              std::cout << '\n' << failures << " CHECK(S) FAILED\n";

    return failures == 0 ? 0 : 1;   // non-zero exit signals failure to make/CI
}
