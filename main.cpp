#include <iostream>
#include <vector>


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
    for(const auto& o:book){
        std::cout << "[id " << o.id << "] " 
        << (o.is_buy ? "BUY ":"SELL ")
        << o.qty
        << " @ "
        << o.price
        << '\n';
    }
}

int main(){
    std::vector<Order> book;
    addOrder(book,Order{1,true,10050,100});
    addOrder(book,Order{2,true,20050,200});
    addOrder(book,Order{3,false,40050,600});
    addOrder(book,Order{4,false,80050,400});
    printBook(book);
    return 0;
}




