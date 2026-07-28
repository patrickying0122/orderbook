#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <deque>
#include <optional>
#include <cassert>

struct Order{
    int id{};
    bool is_buy{};
    long price{};
    int qty{};
};

class OrderBook{

    public:
    // since it's under a class, the method can access the member variables defined in the private field (e.g orders_) and we can just pass in the Order object we want to add to the orderbook.
    void addOrder(const Order& o){
        if (o.is_buy){
            bids_[o.price].push_back(o);
        }
        else{
            asks_[o.price].push_back(o);
        }
    }

    std::optional<long> bestBid() const{
        if(bids_.empty()){
            return std::nullopt;
        }
        return bids_.begin()->first;

    }

    std::optional<long> bestAsk() const{
        if(asks_.empty()){
            return std::nullopt;
        }
        return asks_.begin()->first;
    }

    std::optional<long> spread() const{
        if(asks_.empty() || bids_.empty()){
            return std::nullopt;
        }
        return *bestAsk() - *bestBid();

    }

    void printBook() const{
        std::cout << "price|total qty|#orders \n";
        std::cout<<"--- SELL --- \n";
        for(auto it = asks_.rbegin(); it != asks_.rend(); ++it){
            const auto& price = it -> first;
            const auto& dq = it -> second;
            int total_qty = 0;
            for(const auto& o:dq) total_qty += o.qty;
            std::cout<< price <<" | " << total_qty << " | " << dq.size()<<"\n";

        }
        
        std::cout<<"--- BUY --- \n";
        for(const auto& [price,dq]:bids_){
            int total_qty = 0;
            for(const auto& o: dq){
                total_qty += o.qty;
            }
            std::cout<< price <<" | " << total_qty << " | " << dq.size()<<"\n";
        }

    }

    bool cancelOrder(int id){
        for(auto& [price,dq]:bids_){
            auto it = std::find_if(dq.begin(),dq.end(),[id](const Order& o){return o.id == id;});
            if(it != dq.end()){
                dq.erase(it);
                if(dq.empty()) bids_.erase(price);
                return true;
            }
        }
        for(auto& [price,dq]:asks_){
            auto it = std::find_if(dq.begin(),dq.end(),[id](const Order& o){return o.id == id;});
            if(it != dq.end()){
                dq.erase(it);
                if(dq.empty()) asks_.erase(price);
                return true;
            }
        }
        return false;

    }
    private:
        std::map<long, std::deque<Order>, std::greater<long>> bids_;
        std::map<long,std::deque<Order>> asks_;

};


int main(){
    OrderBook book{};

    // --- EDGE CASE 1: empty book ---
    // No orders added yet: every accessor should report "no value", and
    // printBook must not crash (it iterates, so an empty map => 0 rows).
    assert(!book.bestBid().has_value());
    assert(!book.bestAsk().has_value());
    assert(!book.spread().has_value());

    // Insert prices deliberately OUT of order to prove the maps do the sorting.
    // Order{ id, is_buy, price, qty }.  is_buy=false -> ask (sell), true -> bid (buy).

    // --- asks / sells ---
    // book.addOrder(Order{1, false, 101, 10});   // ask @ 101
    // book.addOrder(Order{2, false, 100, 5});    // ask @ 100  (best ask)
    // book.addOrder(Order{3, false, 102, 7});    // ask @ 102
    // book.addOrder(Order{4, false, 100, 3});    // ask @ 100 again -> level has 2 orders, qty 5+3=8

    // --- bids / buys ---
    book.addOrder(Order{5, true, 98, 12});     // bid @ 98
    book.addOrder(Order{6, true, 99, 4});      // bid @ 99  (best bid)
    book.addOrder(Order{7, true, 97, 8});      // bid @ 97
    book.addOrder(Order{8, true, 99, 6});      // bid @ 99 again -> level has 2 orders, qty 4+6=10

    // --- EDGE CASE 2: one-sided book (bids only, asks still commented out) ---
    // Best bid exists (highest = 99); best ask and spread do not.
    assert(book.bestBid().has_value() && *book.bestBid() == 99);
    assert(!book.bestAsk().has_value());
    assert(!book.spread().has_value());

    if(auto s = book.spread()){ 
        std::cout << "Spread: "<< *s << '\n';
    }
    else{
        std::cout << "No spread";
    }

    // The book may be one-sided here, so bestAsk()/spread() can be empty.
    // value_or(-1) prints -1 instead of dereferencing an empty optional (UB).
    std::cout << "\nbest ask = " << book.bestAsk().value_or(-1)
              << "   best bid = " << book.bestBid().value_or(-1)
              << "   spread = "   << book.spread().value_or(-1) << "\n";

    std::cout << "\nAll main() edge-case asserts passed.\n";
    return 0;
}




