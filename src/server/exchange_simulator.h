#pragma once
#include <vector>
#include <cstdint>

class ExchangeSimulator {
public:
    ExchangeSimulator(uint16_t port, size_t symbols);

    void start();

    void run();

    void set_tick_rate(uint32_t rate);

private:
    void generate_tick(uint16_t symbol);

    uint16_t port;
    size_t num_symbols;
    uint32_t tick_rate{10000};
};
