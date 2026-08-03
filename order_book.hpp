#pragma once 

#include <map>
#include <deque>
#include <optional>
#include <vector>
#include <algorithm>
#include <list>
#include <unordered_map>

struct Order{
    int id{};
    bool is_buy{};
    long price{};
    int qty{};
    bool is_market{};
};

struct Trade{
    int buy_id{};
    int sell_id{};
    long price{};
    int qty{};
};

struct Locator{
    bool is_buy{};
    long price {};
    std::list<Order>::iterator it;
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

    const std::vector<Trade>& tradeAccessor() const;

    private:
        std::map<long, std::list<Order>, std::greater<long>> bids_;
        std::map<long, std::list<Order>> asks_;
        std::vector<Trade> trade;
        std::unordered_map<int,Locator> index_;

        template <typename LevelMap,typename Crosses>
        void matchAgainst(LevelMap& opposite, Order& o, Crosses crosses){
            while(o.qty > 0 && !opposite.empty() && crosses(opposite.begin()->first)){
                auto lvl = opposite.begin();
                // get the list of orders sitting at that price level
                Order& resting = lvl -> second.front();
                int fill = std::min(o.qty, resting.qty);
                if(o.is_buy) trade.push_back(Trade{o.id,resting.id,resting.price,fill});
                else trade.push_back(Trade{resting.id,o.id,resting.price,fill});
                
                o.qty -= fill;
                resting.qty -= fill;
                if(resting.qty == 0){
                    // Drop the index entry BEFORE pop_front: `resting` is a reference
                    // into the node that pop_front destroys, so reading its id after
                    // would be a use-after-free.
                    index_.erase(resting.id);
                    lvl -> second.pop_front();
                    if(lvl -> second.empty()){
                        lvl = opposite.erase(lvl);
                    }
                }
            }
        }

};
