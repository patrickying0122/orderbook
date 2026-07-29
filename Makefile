# ---- configuration ----
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra

# order_book.cpp is shared by every binary (it holds the class implementation).
CORE := order_book.cpp

# ---- targets ----

# Default target (runs when you type just `make`): the demo program.
ob: main.cpp $(CORE) order_book.hpp
	$(CXX) $(CXXFLAGS) main.cpp $(CORE) -o ob

# Test binary. Needs tests.cpp (has its own main()); does NOT link main.cpp.
test: tests.cpp $(CORE) order_book.hpp
	$(CXX) $(CXXFLAGS) tests.cpp $(CORE) -o test

# Benchmark binary. -O2 optimization is added here ONLY, per the plan.
bench: bench.cpp $(CORE) order_book.hpp
	$(CXX) $(CXXFLAGS) -O2 bench.cpp $(CORE) -o bench

# Remove all built binaries.
clean:
	rm -f ob test bench

.PHONY: clean
