#include <chrono>
#include <iostream>
#include <cstring>

#include "../src/common/protocol.h"
#include "../include/cache.h"
#include "../include/parser.h"

static uint32_t checksum(const char *data, size_t len) {
    uint32_t x = 0;
    for (size_t i = 0; i < len; i++) x ^= data[i];
    return x;
}

int main() {
    Cache cache(100);
    Parser parser(cache);

    const int N = 1'000'000;

    char buffer[128];
    size_t len = 0;

    Header h{};
    h.type = MsgType::TRADE;
    h.seq = 1;
    h.timestamp = 123456;
    h.symbol = 1;

    Trade t{};
    t.price = 100.5;
    t.qty = 10;

    memcpy(buffer, &h, sizeof(h));
    memcpy(buffer + sizeof(h), &t, sizeof(t));
    len = sizeof(h) + sizeof(t);

    uint32_t cs = checksum(buffer, len);
    memcpy(buffer + len, &cs, 4);
    len += 4;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        parser.feed(buffer, len);
    }

    auto end = std::chrono::high_resolution_clock::now();

    double seconds =
            std::chrono::duration<double>(end - start).count();

    std::cout << "Parser throughput: "
            << (N / seconds) << " msg/sec\n";

    return 0;
}
