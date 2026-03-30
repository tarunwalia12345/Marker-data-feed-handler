#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <cstring>
#include "cache.h"

#define CLEAR "\033[H"
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

static const char* SYMBOLS[] = {
    "RELIANCE","TCS","INFY","HDFC","ICICIBANK",
    "SBIN","LT","AXISBANK","WIPRO","HCLTECH"
};

struct Stats {
    std::atomic<uint64_t> messages{0};
    std::atomic<uint64_t> updates{0};
    std::atomic<uint64_t> seq_gaps{0};
};

Stats stats;

class LatencyTracker {
public:
    void record(uint64_t ns) {
        size_t i = index.fetch_add(1, std::memory_order_relaxed);
        buffer[i % MAX] = ns;
    }

    void get(uint64_t& p50, uint64_t& p99, uint64_t& p999) {
        size_t n = std::min(index.load(std::memory_order_relaxed), MAX);

        if (n == 0) {
            p50 = p99 = p999 = 0;
            return;
        }

        std::vector<uint64_t> tmp(buffer.begin(), buffer.begin() + n);
        std::sort(tmp.begin(), tmp.end());

        p50  = tmp[n * 50 / 100];
        p99  = tmp[n * 99 / 100];
        p999 = tmp[n * 999 / 1000];
    }

    void reset() {
        index.store(0, std::memory_order_relaxed);
    }

private:
    static constexpr size_t MAX = 100000; // ✅ FIXED
    std::vector<uint64_t> buffer = std::vector<uint64_t>(MAX);
    std::atomic<size_t> index{0};
};

LatencyTracker latency;

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
    uint64_t rate = seconds > 0 ? messages / seconds : 0;

    std::vector<Row> rows;

    for (int i = 0; i < 100; i++) {
        auto s = cache.get(i);
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

    std::partial_sort(
        rows.begin(),
        rows.begin() + topN,
        rows.end(),
        [](const Row& a, const Row& b) {
            return a.updates > b.updates;
        }
    );

    if (rows.size() > topN) rows.resize(topN);

    uint64_t p50, p99, p999;
    latency.get(p50, p99, p999);

    std::cout << CLEAR;

    std::cout << "=== NSE Market Data Feed Handler ===\n";
    std::cout << "Connected to: localhost:9876\n";
    std::cout << "Uptime: " << (int)seconds
              << "s | Messages: " << messages
              << " | Rate: " << rate << " msg/s\n\n";

    std::cout << std::left
              << std::setw(12) << "Symbol"
              << std::setw(12) << "Bid"
              << std::setw(12) << "Ask"
              << std::setw(12) << "LTP"
              << std::setw(12) << "Volume"
              << std::setw(10) << "Chg%"
              << std::setw(12) << "Updates"
              << "\n";

    std::cout << "-------------------------------------------------------------------\n";

    for (const auto& r : rows) {

        char name_buf[16];
        if (r.symbol < 10) {
            std::strncpy(name_buf, SYMBOLS[r.symbol], sizeof(name_buf));
            name_buf[sizeof(name_buf) - 1] = '\0';
        } else {
            std::snprintf(name_buf, sizeof(name_buf), "SYM_%d", r.symbol);
        }

        double mid = (r.bid + r.ask) / 2.0;
        double chg = mid > 0 ? ((r.ltp - mid) / mid) * 100 : 0;

        const char* color = (chg >= 0) ? GREEN : RED;

        std::cout << std::setw(12) << name_buf
                  << std::setw(12) << r.bid
                  << std::setw(12) << r.ask
                  << color << std::setw(12) << r.ltp << RESET
                  << std::setw(12) << r.volume
                  << std::setw(10) << std::fixed << std::setprecision(2) << chg
                  << std::setw(12) << r.updates
                  << "\n";
    }

    std::cout << "\nStatistics:\n";
    std::cout << "Parser Throughput: " << rate << " msg/s\n";
    std::cout << "Latency: p50=" << p50
              << "ns p99=" << p99
              << "ns p999=" << p999 << "ns\n";
    std::cout << "Sequence Gaps: " << stats.seq_gaps.load() << "\n";
    std::cout << "Cache Updates: " << stats.updates.load() << "\n";

    std::cout << "\nPress 'q' to quit, 'r' to reset stats\n";
}