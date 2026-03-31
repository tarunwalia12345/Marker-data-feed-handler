#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <cstring>
#include <signal.h>

#include "cache.h"
#include "latency_tracker.h"

extern LatencyTracker latency;

extern struct Stats {
    std::atomic<uint64_t> messages;
    std::atomic<uint64_t> updates;
    std::atomic<uint64_t> seq_gaps;
} stats;

#define CLEAR "\033[H"
#define CLEAR_ALL "\033[2J"
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

void handle_resize(int) {
    std::cout << CLEAR_ALL;
}


void render(Cache &cache, uint64_t messages) {

    constexpr int MAX_SYMBOLS = 100;

    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration<double>(now - start).count();
    double rate = seconds > 0 ? messages / seconds : 0;

    struct Row {
        std::string name;
        double bid, ask, ltp;
        uint32_t volume;
        uint64_t updates;
        double change;
    };

    static const char* SYMBOLS[] = {
        "RELIANCE","TCS","INFY","HDFC","ICICIBANK","SBIN","WIPRO","LT",
        "AXISBANK","HCLTECH","MARUTI","BAJFINANCE","ITC","KOTAKBANK",
        "ADANIENT","ASIANPAINT","SUNPHARMA","ULTRACEMCO","TITAN","ONGC"
    };

    static std::vector<double> base(MAX_SYMBOLS, 0);

    std::vector<Row> rows;
    rows.reserve(MAX_SYMBOLS);

    // ===== BUILD DATA =====
    for (int i = 0; i < MAX_SYMBOLS; i++) {

        auto s = cache.get(i);

        if (s.last_price < 10 || s.updates < 5) continue;

        if (base[i] == 0) base[i] = s.last_price;

        double chg = ((s.last_price - base[i]) / base[i]) * 100.0;

        std::string name = (i < 20) ? SYMBOLS[i] : ("SYM" + std::to_string(i));

        rows.push_back({
            name,
            s.best_bid,
            s.best_ask,
            s.last_price,
            s.last_qty,
            s.updates,
            chg
        });
    }

    std::sort(rows.begin(), rows.end(),
        [](const Row &a, const Row &b) {
            return a.updates > b.updates;
        });

    if (rows.size() > 20) rows.resize(20);

    auto lat = latency.get_stats();

    std::cout << CLEAR_ALL << CLEAR;

    std::cout << "=== NSE Market Data Feed Handler ===\n";
    std::cout << "Connected to: localhost:9876\n";

    int hrs = static_cast<int>(seconds) / 3600;
    int mins = (static_cast<int>(seconds) % 3600) / 60;
    int secs = static_cast<int>(seconds) % 60;

    std::cout << "Uptime: "
              << std::setw(2) << std::setfill('0') << hrs << ":"
              << std::setw(2) << mins << ":"
              << std::setw(2) << secs;

    std::cout << std::setfill(' ');

    std::cout << " | Messages: " << messages
              << " | Rate: " << static_cast<uint64_t>(rate) << " msg/s\n\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << std::left
              << std::setw(12) << "Symbol"
              << std::right
              << std::setw(12) << "Bid"
              << std::setw(12) << "Ask"
              << std::setw(12) << "LTP"
              << std::setw(12) << "Volume"
              << std::setw(10) << "Chg%"
              << std::setw(12) << "Updates"
              << "\n";

    std::cout << "-------------------------------------------------------------------------------\n";

    for (const auto &r : rows) {

        const char *color = (r.change >= 0) ? GREEN : RED;

        std::cout << std::left << std::setw(12) << r.name
                  << std::right
                  << std::setw(12) << r.bid
                  << std::setw(12) << r.ask
                  << std::setw(12) << r.ltp
                  << std::setw(12) << r.volume
                  << color << std::setw(10) << r.change << RESET
                  << std::setw(12) << r.updates
                  << "\n";
    }

    std::cout << "\n[Top 20 symbols by update frequency]\n\n";

    std::cout << "Statistics:\n";

    std::cout << "Parser Throughput: "
              << static_cast<uint64_t>(rate) << " msg/s\n";

    std::cout << "End-to-End Latency: "
              << "p50=" << lat.p50 / 1000 << "us "
              << "p99=" << lat.p99 / 1000 << "us "
              << "p999=" << lat.p999 / 1000 << "us\n";

    std::cout << "Sequence Gaps: " << stats.seq_gaps.load() << "\n";
    std::cout << "Cache Updates: " << messages << "\n";

    std::cout << "\nPress 'q' to quit, 'r' to reset stats\n";
}