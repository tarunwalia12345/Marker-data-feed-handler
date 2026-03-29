#include <iostream>
#include "cache.h"

void render(Cache& cache) {
    std::cout << "\033[2J\033[H"; // clear screen
    std::cout << "=== NSE Market Data Feed ===\n";

    for (int i = 0; i < 10; i++) {
        auto s = cache.get(i);

        std::cout << "Sym " << i
                  << " Bid: " << s.best_bid
                  << " Ask: " << s.best_ask
                  << " LTP: " << s.last_price
                  << " Updates: " << s.updates
                  << "\n";
    }
}