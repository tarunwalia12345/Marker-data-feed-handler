#pragma once
#include <vector>
#include <atomic>
#include <cstdint>


struct alignas(64) MarketState {
    std::atomic<double> best_bid{0};
    std::atomic<double> best_ask{0};
    std::atomic<uint32_t> bid_qty{0};
    std::atomic<uint32_t> ask_qty{0};

    std::atomic<double> last_price{0};
    std::atomic<uint32_t> last_qty{0};

    std::atomic<uint64_t> last_update{0};
    std::atomic<uint64_t> updates{0};
};


struct MarketSnapshot {
    double best_bid;
    double best_ask;
    uint32_t bid_qty;
    uint32_t ask_qty;

    double last_price;
    uint32_t last_qty;

    uint64_t last_update;
    uint64_t updates;
};

class Cache {
    std::vector<MarketState> data;

public:
    explicit Cache(size_t n);

    void update_trade(uint16_t sym, double price, uint32_t qty);
    void update_quote(uint16_t sym, double bid, uint32_t bq, double ask, uint32_t aq);


    MarketSnapshot get(uint16_t sym) const;
};