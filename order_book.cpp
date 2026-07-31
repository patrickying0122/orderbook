#include "order_book.hpp"
#include <iostream>
#include <algorithm>


    // std::map<long, std::deque<Order>, std::greater<long>> bids_;
    // std::map<long,std::deque<Order>> asks_;
void OrderBook::addOrder(Order& o){
    if (o.is_buy){
        //the third parameter is a lambda expression that returns the lvl that matches the cross price.
        matchAgainst(asks_,o,[&o](long lvl){return o.is_market || o.price >= lvl;});
        if(o.qty > 0 && !o.is_market) bids_[o.price].push_back(o);
    }
    else{
        matchAgainst(bids_,o,[&o](long lvl){return o.is_market || o.price <= lvl;});
        if (o.qty >0 && !o.is_market) asks_[o.price].push_back(o);
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

std::vector<Trade> OrderBook::tradeAccessor() const{
    return trade;
}