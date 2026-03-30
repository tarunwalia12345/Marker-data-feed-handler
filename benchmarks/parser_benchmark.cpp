#include <chrono>
#include <iostream>
#include <cstring>

#include "../src/common/protocol.h"
#include "../include/cache.h"
#include "../include/parser.h"

int main() {
    Cache cache(100);
    Parser parser(cache);

    const int N = 1'000'000;

    char buffer[64] = {0};

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        parser.feed(buffer, sizeof(buffer));
    }

    auto end = std::chrono::high_resolution_clock::now();

    double seconds =
        std::chrono::duration<double>(end - start).count();

    std::cout << "Parser throughput: "
              << (N / seconds) << " msg/sec\n";

    return 0;
}