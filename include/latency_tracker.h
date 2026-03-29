#pragma once
#include <array>
#include <atomic>
#include <cstdint>

class LatencyTracker {
    static constexpr int BUCKETS = 100;

    std::array<std::atomic<uint64_t>, BUCKETS> histogram{};
    std::atomic<uint64_t> total_samples{0};
    std::atomic<uint64_t> sum{0};

    int bucket_index(uint64_t ns) const;

public:
    void record(uint64_t latency_ns);

    struct Stats {
        uint64_t min;
        uint64_t max;
        uint64_t mean;
        uint64_t p50;
        uint64_t p95;
        uint64_t p99;
        uint64_t p999;
        uint64_t sample_count;
    };

    Stats get_stats() const;
};