#include "order_book.hpp"
#include <iostream>
#include <algorithm>


    // std::map<long, std::deque<Order>, std::greater<long>> bids_;
    // std::map<long,std::deque<Order>> asks_;
void OrderBook::addOrder(Order& o){
    if (o.is_buy){
        //the third parameter is a lambda expression that returns the lvl that matches the cross price.
        matchAgainst(asks_,o,[&o](long lvl){return o.is_market || o.price >= lvl;});
        if(o.qty > 0 && !o.is_market) {
            bids_[o.price].push_back(o);
            auto it = std::prev(bids_[o.price].end());
            index_[o.id] = Locator{true,o.price,it};
        }
    }
    else{
        matchAgainst(bids_,o,[&o](long lvl){return o.is_market || o.price <= lvl;});
        if (o.qty >0 && !o.is_market){
            asks_[o.price].push_back(o);
            auto it = std::prev(asks_[o.price].end());
            index_[o.id] = Locator{false,o.price,it};
        }
    }
}

std::optional<long> OrderBook:: bestBid() const{
    if(bids_.empty()){
        return std::nullopt;
    }
    return bids_.begin()->first;

}

std::optional<long> OrderBook:: bestAsk() const{
    if(asks_.empty()){
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::optional<long> OrderBook::spread() const{
    if(asks_.empty() || bids_.empty()){
        return std::nullopt;
    }
    return *bestAsk() - *bestBid();

}

void OrderBook::printBook() const{
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
bool OrderBook::cancelOrder(int id){
    // the find method used will return a iterator into the unordered_map, so 
    // it -> {first: 42 (the id), second:Locator{...}}
    auto hit = index_.find(id);
    if(hit == index_.end()) return false;
    const Locator& loc = hit -> second;

    if(loc.is_buy){
        //lvl will be a iterator into the bids_ and lvl->second would be the list at that price level
        auto lvl = bids_.find(loc.price);
        if (lvl == bids_.end()) return false;
        lvl->second.erase(loc.it);
        if (lvl->second.empty()) bids_.erase(lvl);
        index_.erase(hit);
        return true;
    }
    else{
        auto lvl = asks_.find(loc.price);
        if (lvl == asks_.end()) return false;
        lvl -> second.erase(loc.it);
        if (lvl->second.empty()) asks_.erase(lvl);
        index_.erase(hit);
        return true;
    }

    return false;
}


const std::vector<Trade>& OrderBook::tradeAccessor() const{
    return trade;
}