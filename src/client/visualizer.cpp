#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <cstring>

#include "cache.h"
#include "latency_tracker.h"

// ✅ FIX: ONLY DECLARE, DO NOT DEFINE
extern LatencyTracker latency;

extern struct Stats {
    std::atomic<uint64_t> messages;
    std::atomic<uint64_t> updates;
    std::atomic<uint64_t> seq_gaps;
} stats;

#define CLEAR "\033[H"
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

struct Row {
    int symbol;
    double bid, ask, ltp;
    uint32_t volume;
    uint64_t updates;
};

void render(Cache& cache, uint64_t messages) {

    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration<double>(now - start).count();
    double rate = seconds > 0 ? static_cast<double>(messages) / seconds : 0.0;

    std::vector<Row> rows;

    for (int i = 0; i < 100; i++) {
        auto s = cache.get(static_cast<uint16_t>(i));

        if (s.updates == 0) continue;

        rows.push_back({
            i,
            s.best_bid,
            s.best_ask,
            s.last_price,
            s.last_qty,
            s.updates
        });
    }

    size_t topN = std::min<size_t>(20, rows.size());

    std::partial_sort(rows.begin(), rows.begin() + topN, rows.end(),
        [](const Row& a, const Row& b) {
            return a.updates > b.updates;
        });

    if (rows.size() > topN) rows.resize(topN);

    auto lat = latency.get_stats();

    std::cout << CLEAR;

    std::cout << "=== NSE Market Data Feed Handler ===\n";
    std::cout << "Messages: " << messages
              << " | Rate: " << rate << "\n\n";

    for (const auto& r : rows) {
        const char* color = (r.ltp >= r.bid) ? GREEN : RED;

        std::cout << std::setw(6) << r.symbol
                  << std::setw(10) << r.bid
                  << std::setw(10) << r.ask
                  << color << std::setw(10) << r.ltp << RESET
                  << std::setw(10) << r.volume
                  << std::setw(10) << r.updates
                  << "\n";
    }

    std::cout << "\nLatency p50=" << lat.p50
              << " p99=" << lat.p99
              << " p999=" << lat.p999 << "\n";

    std::cout << "Seq gaps: " << stats.seq_gaps.load() << "\n";
}