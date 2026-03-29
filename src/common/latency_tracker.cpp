#include "latency_tracker.h"

int LatencyTracker::bucket_index(uint64_t ns) const {
    uint64_t us = ns / 1000;

    int idx = 0;
    while (us > 1 && idx < BUCKETS - 1) {
        us >>= 1;
        idx++;
    }
    return idx;
}

void LatencyTracker::record(uint64_t ns) {
    int idx = bucket_index(ns);

    histogram[idx].fetch_add(1, std::memory_order_relaxed);
    total_samples.fetch_add(1, std::memory_order_relaxed);
    sum.fetch_add(ns, std::memory_order_relaxed);
}

LatencyTracker::Stats LatencyTracker::get_stats() const {
    Stats s{};
    s.sample_count = total_samples.load();

    if (s.sample_count == 0) return s;

    s.mean = sum.load() / s.sample_count;

    uint64_t p50_t = s.sample_count * 50 / 100;
    uint64_t p95_t = s.sample_count * 95 / 100;
    uint64_t p99_t = s.sample_count * 99 / 100;
    uint64_t p999_t = s.sample_count * 999 / 1000;

    uint64_t cumulative = 0;

    for (int i = 0; i < BUCKETS; i++) {
        cumulative += histogram[i].load();

        uint64_t val = (1ULL << i) * 1000;

        if (!s.p50 && cumulative >= p50_t) s.p50 = val;
        if (!s.p95 && cumulative >= p95_t) s.p95 = val;
        if (!s.p99 && cumulative >= p99_t) s.p99 = val;
        if (!s.p999 && cumulative >= p999_t) s.p999 = val;

        if (!s.min && histogram[i] > 0) s.min = val;
        if (histogram[i] > 0) s.max = val;
    }

    return s;
}