#include <iostream>
#include "order_book.hpp"

// Report EVERY failed check (unlike assert, which dies on the first).
// __LINE__ = the source line of the failing CHECK; #c = the condition as text.
#define CHECK(c) do { if(!(c)) { std::cout << "FAIL " << __LINE__ << ": " << #c << '\n'; ++failures; } } while(0)

// Incoming BUY sweeps two ask levels: eats level 90 whole, partially fills
// level 95. Checks multi-level walking, price improvement (each trade prints
// at the RESTING level's price, not the buy's 95), and that the emptied
// level is erased.
void test_buy_sweeps_two_ask_levels(int& failures){
    OrderBook book;
    Order s1 {1, false, 90, 100};   // ask level 1: 100 @ 90
    Order s2 {2, false, 95,  50};   // ask level 2:  50 @ 95
    Order b  {3, true,  95, 130};   // incoming buy: 130 @ 95
    book.addOrder(s1);
    book.addOrder(s2);
    book.addOrder(b);

    const auto& trades = book.tradeAccessor();
    CHECK(trades.size() == 2);
    if (trades.size() == 2) {
        CHECK(trades[0].buy_id  == 3);
        CHECK(trades[0].sell_id == 1);
        CHECK(trades[0].price   == 90);    // resting level's price, not the buy's 95
        CHECK(trades[0].qty     == 100);   // level 90 fully swept

        CHECK(trades[1].buy_id  == 3);
        CHECK(trades[1].sell_id == 2);
        CHECK(trades[1].price   == 95);
        CHECK(trades[1].qty     == 30);    // 130 - 100 leftover eats part of level 95
    }

    CHECK(book.bestAsk().has_value() && book.bestAsk().value() == 95); // level 90 erased, 95 survives with 20
    CHECK(!book.bestBid().has_value());    // 100 + 30 = 130, buy fully consumed, nothing rests
}

// Mirror of the above on the other side: incoming SELL sweeps two bid levels.
// bids_ sorts high-to-low, so the 100 level is hit first. Also checks that
// buy_id/sell_id land in the right fields now that the INCOMING order is the
// seller and the RESTING orders are the buyers.
void test_sell_sweeps_two_bid_levels(int& failures){
    OrderBook book;
    Order b1 {1, true, 100, 100};   // bid level 1: 100 @ 100 (best bid)
    Order b2 {2, true,  95,  50};   // bid level 2:  50 @ 95
    Order s  {3, false, 95, 130};   // incoming sell: 130 @ 95
    book.addOrder(b1);
    book.addOrder(b2);
    book.addOrder(s);

    const auto& trades = book.tradeAccessor();
    CHECK(trades.size() == 2);
    if (trades.size() == 2) {
        CHECK(trades[0].buy_id  == 1);     // resting order is the BUYER here
        CHECK(trades[0].sell_id == 3);     // incoming order is the SELLER
        CHECK(trades[0].price   == 100);   // seller asked 95, got 100: price improvement
        CHECK(trades[0].qty     == 100);   // level 100 fully swept

        CHECK(trades[1].buy_id  == 2);
        CHECK(trades[1].sell_id == 3);
        CHECK(trades[1].price   == 95);
        CHECK(trades[1].qty     == 30);    // 130 - 100 leftover eats part of level 95
    }

    CHECK(book.bestBid().has_value() && book.bestBid().value() == 95); // level 100 erased, 95 survives with 20
    CHECK(!book.bestAsk().has_value());    // sell fully consumed, nothing rests
}

// MARKET buy: no price, so it crosses every level regardless. Sweeps ask
// level 90 whole and partially fills level 95. Its price field is deliberately
// junk (0) to prove the predicate never reads it.
void test_market_buy(int& failures){
    OrderBook book;
    Order s1 {1, false, 90, 100};        // ask level 1: 100 @ 90
    Order s2 {2, false, 95,  50};        // ask level 2:  50 @ 95
    Order m  {3, true,   0, 120, true};  // market buy: 120, price 0 is ignored
    book.addOrder(s1);
    book.addOrder(s2);
    book.addOrder(m);

    const auto& trades = book.tradeAccessor();
    CHECK(trades.size() == 2);
    if (trades.size() == 2) {
        CHECK(trades[0].buy_id  == 3);
        CHECK(trades[0].sell_id == 1);
        CHECK(trades[0].price   == 90);    // trades at the resting price, not the market order's 0
        CHECK(trades[0].qty     == 100);   // level 90 fully swept

        CHECK(trades[1].buy_id  == 3);
        CHECK(trades[1].sell_id == 2);
        CHECK(trades[1].price   == 95);
        CHECK(trades[1].qty     == 20);    // 120 - 100 leftover eats part of level 95
    }

    CHECK(book.bestAsk().has_value() && book.bestAsk().value() == 95); // 30 left at 95
    CHECK(!book.bestBid().has_value());    // a market order never rests
}

int main(){
    int failures = 0;

    test_buy_sweeps_two_ask_levels(failures);
    test_sell_sweeps_two_bid_levels(failures);
    test_market_buy(failures);

    if(failures == 0) std::cout << "\nALL TESTS PASSED\n";
    else              std::cout << '\n' << failures << " CHECK(S) FAILED\n";

    return failures == 0 ? 0 : 1;   // non-zero exit signals failure to make/CI
}
