#include <iostream>
#include <vector>
#include <algorithm>


struct Order{
    int id;
    bool is_buy;
    long price;
    int qty;
};


void addOrder(std::vector<Order>& book, const Order& o){
    book.push_back(o);
}

void printBook(const std::vector<Order>& book){
    std::vector<Order> buys,sells;
    for (const auto& order:book){
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

void cancelOrder(std::vector<Order>& book, int id){
    book.erase(
        std::remove_if(book.begin(),book.end(),[id](const Order& o){return o.id == id;}),book.end()
    );
}

int main(){
    std::vector<Order> book;
    addOrder(book,Order{1,true,10050,100});
    addOrder(book,Order{2,true,20050,200});
    addOrder(book,Order{3,false,40050,600});
    addOrder(book,Order{4,false,80050,400});
    cancelOrder(book,2);
    printBook(book);
    return 0;
}




