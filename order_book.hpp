#pragma once 

#include <map>
#include <deque>
#include <optional>
#include <vector>

struct Order{
    int id{};
    bool is_buy{};
    long price{};
    int qty{};
};

struct Trade{
    int buy_id{};
    int sell_id{};
    long price{};
    int qty;
};

class OrderBook{

    public:
    // since it's under a class, the method can access the member variables defined in the private field (e.g orders_) and we can just pass in the Order object we want to add to the orderbook.
    void addOrder(Order& o);

    std::optional<long> bestBid() const;

    std::optional<long> bestAsk() const;

    std::optional<long> spread() const;

    void printBook() const;

    bool cancelOrder(int id);

    std::vector<Trade> tradeAccessor() const;

    private:
        std::map<long, std::deque<Order>, std::greater<long>> bids_;
        std::map<long,std::deque<Order>> asks_;
        std::vector<Trade> trade;

};
