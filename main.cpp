#include <iostream>
#include <vector>
#include <algorithm>


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
        orders_.push_back(o);
    }

    // the method for printing out the orders should not modify the orderbook itself, so we can make it a const method 
    void printBook() const{
        std::vector<Order> buys,sells;
        for (const auto& order:orders_){
            if (order.is_buy) buys.push_back(order);
            else sells.push_back(order);
        }
        
        std::sort(buys.begin(),buys.end(),[](const Order& a,const Order& b){return a.price > b.price;});

        std::sort(sells.begin(),sells.end(),[](const Order& a, const Order& b){return a.price > b.price;});

        std::cout<<"--- SELL --- \n";
        for(const auto& o:sells){
            std::cout << "[id " << o.id << "] " 
            << "SELL "
            << o.qty
            << " @ "
            << o.price
            << '\n';
        }

        std::cout<<"--- BUY ---\n";
        for(const auto& o:buys){
            std::cout << "[id " << o.id << "] " 
            << "BUY "
            << o.qty
            << " @ "
            << o.price
            << '\n';
        }
    }

    bool cancelOrder(int id){
        auto it = std::find_if(orders_.begin(),orders_.end(),[id](const Order&o){return o.id == id;});
        
        if (it != orders_.end()){
            orders_.erase(it);
            return true;
        }
        return false;
    }
    private:
        std::vector<Order> orders_;


};


int main(){
    OrderBook book{};
    book.addOrder(Order{1,true,10050,100});
    book.addOrder(Order{2,true,20050,100});
    book.addOrder(Order{3,false,10050,600});
    book.addOrder(Order{4,false,80050,400});
    book.cancelOrder(2);
    book.printBook();

    return 0;
}




