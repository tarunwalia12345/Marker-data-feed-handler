#include "cache.h"

Cache::Cache(size_t n) : data(n) {}

void Cache::update_trade(uint16_t s, double price, uint32_t qty) {
    data[s].last_price.store(price, std::memory_order_release);
    data[s].last_qty.store(qty, std::memory_order_relaxed);
    data[s].updates.fetch_add(1, std::memory_order_relaxed);
}

void Cache::update_quote(uint16_t s, double bid, uint32_t bq, double ask, uint32_t aq) {
    data[s].best_bid.store(bid, std::memory_order_release);
    data[s].best_ask.store(ask, std::memory_order_release);
    data[s].bid_qty.store(bq, std::memory_order_relaxed);
    data[s].ask_qty.store(aq, std::memory_order_relaxed);
    data[s].updates.fetch_add(1, std::memory_order_relaxed);
}

MarketSnapshot Cache::get(uint16_t s) const {
    MarketSnapshot snap;

    snap.best_bid = data[s].best_bid.load(std::memory_order_acquire);
    snap.best_ask = data[s].best_ask.load(std::memory_order_acquire);
    snap.bid_qty = data[s].bid_qty.load(std::memory_order_relaxed);
    snap.ask_qty = data[s].ask_qty.load(std::memory_order_relaxed);
    snap.last_price = data[s].last_price.load(std::memory_order_acquire);
    snap.last_qty = data[s].last_qty.load(std::memory_order_relaxed);
    snap.last_update = data[s].last_update.load(std::memory_order_relaxed);
    snap.updates = data[s].updates.load(std::memory_order_relaxed);
    return snap;
}