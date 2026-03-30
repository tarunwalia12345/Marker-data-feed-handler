#include "../../include/cache.h"
#include <chrono>
#include <stdexcept>

Cache::Cache(size_t n) : data(n) {
    if (n == 0) throw std::invalid_argument("Cache size must be > 0");
}

void Cache::update_trade(uint16_t sym, double price, uint32_t qty) {
    auto& s = data[sym];

    uint64_t v = s.version.load(std::memory_order_relaxed);
    s.version.store(v + 1, std::memory_order_release);

s.last_price.store(static_cast<int64_t>(price * 10000), std::memory_order_relaxed);

    s.last_qty.store(qty, std::memory_order_relaxed);

    s.last_update.store(
        std::chrono::steady_clock::now().time_since_epoch().count(),
        std::memory_order_relaxed
    );

    s.updates.fetch_add(1, std::memory_order_relaxed);

    s.version.store(v + 2, std::memory_order_release);
}

void Cache::update_quote(uint16_t sym, double bid, uint32_t bq,
                         double ask, uint32_t aq) {
    auto& s = data[sym];

    uint64_t v = s.version.load(std::memory_order_relaxed);
    s.version.store(v + 1, std::memory_order_release);

s.best_bid.store(static_cast<int64_t>(bid * 10000), std::memory_order_relaxed);
    s.bid_qty.store(bq, std::memory_order_relaxed);

s.best_ask.store(static_cast<int64_t>(ask * 10000), std::memory_order_relaxed);
    s.ask_qty.store(aq, std::memory_order_relaxed);

    s.last_update.store(
        std::chrono::steady_clock::now().time_since_epoch().count(),
        std::memory_order_relaxed
    );

    s.updates.fetch_add(1, std::memory_order_relaxed);

    s.version.store(v + 2, std::memory_order_release);
}

void Cache::updateBid(uint16_t sym, double bid, uint32_t qty) {
    auto snap = get(sym); // read current state

    update_quote(sym,
                 bid, qty,
                 snap.best_ask, snap.ask_qty);
}

void Cache::updateAsk(uint16_t sym, double ask, uint32_t qty) {
    auto snap = get(sym);

    update_quote(sym,
                 snap.best_bid, snap.bid_qty,
                 ask, qty);
}

MarketSnapshot Cache::get(uint16_t sym) const {
    const auto& s = data[sym];
    MarketSnapshot snap;

    while (true) {
        uint64_t v1 = s.version.load(std::memory_order_acquire);
        if (v1 & 1) continue;

        snap.best_bid = static_cast<double>(s.best_bid.load(std::memory_order_relaxed)) / 10000.0;
        snap.best_ask = static_cast<double>(s.best_ask.load(std::memory_order_relaxed)) / 10000.0;

        snap.bid_qty = s.bid_qty.load(std::memory_order_relaxed);
        snap.ask_qty = s.ask_qty.load(std::memory_order_relaxed);
        snap.last_price = static_cast<double>(s.last_price.load(std::memory_order_relaxed)) / 10000.0;
        snap.last_qty = s.last_qty.load(std::memory_order_relaxed);

        snap.last_update = s.last_update.load(std::memory_order_relaxed);
        snap.updates = s.updates.load(std::memory_order_relaxed);

        uint64_t v2 = s.version.load(std::memory_order_acquire);
        if (v1 == v2) break;
    }

    return snap;
}