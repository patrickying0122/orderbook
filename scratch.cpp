#include <iostream>
#include <map>
#include <deque>

int main() {
    int local = 42;                 // lives on the STACK
    int* heaped = new int(7);       // lives on the HEAP

    std::map<long, std::deque<int>> m;
    m[100].push_back(5);            // map node + deque block: also HEAP

    std::cout << "stack local:   " << &local        << "\n";
    std::cout << "heap int:      " << heaped        << "\n";
    std::cout << "deque element: " << &m[100][0]    << "\n";

    delete heaped;
    return 0;
}
