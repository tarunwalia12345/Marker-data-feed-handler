#include <iostream>
#include <chrono>
#include "../include/cache.h"

int main() {
    Cache cache(100);

    const int N = 1'000'000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        cache.update_trade(1, 100.0 + i, i);
    }

    auto end = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();

    std::cout << "Cache updates/sec: " << (N / seconds) << std::endl;

    return 0;
}