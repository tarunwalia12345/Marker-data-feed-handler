#include "latency_tracker.h"

// ===============================
// BUCKET MAPPING (log scale)
// ===============================
size_t LatencyTracker::bucket_index(uint64_t ns) const {
    size_t idx = 0;

    // convert to microseconds for stability
    uint64_t us = ns / 1000;

    while (us > 1 && idx + 1 < BUCKETS) {
        us >>= 1;
        idx++;
    }

    return idx;
}

// ===============================
// RECORD (HOT PATH)
// ===============================
void LatencyTracker::record(uint64_t latency_ns) {
    size_t idx = bucket_index(latency_ns);

    histogram[idx].fetch_add(1, std::memory_order_relaxed);
    total_samples.fetch_add(1, std::memory_order_relaxed);
    sum.fetch_add(latency_ns, std::memory_order_relaxed);
}

// ===============================
// GET STATS
// ===============================
LatencyTracker::Stats LatencyTracker::get_stats() const {
    Stats s{};

    uint64_t total = total_samples.load(std::memory_order_relaxed);
    s.sample_count = total;

    if (total == 0) return s;

    s.mean = sum.load(std::memory_order_relaxed) / total;

    // percentile targets
    uint64_t p50_t = total * 50 / 100;
    uint64_t p95_t = total * 95 / 100;
    uint64_t p99_t = total * 99 / 100;
    uint64_t p999_t = total * 999 / 1000;

    uint64_t cumulative = 0;

    bool min_set = false;

    for (size_t i = 0; i < BUCKETS; i++) {
        uint64_t count = histogram[i].load(std::memory_order_relaxed);
        if (count == 0) continue;

        // approximate latency value (reverse log scale)
        uint64_t value = (1ULL << i) * 1000; // back to ns

        if (!min_set) {
            s.min = value;
            min_set = true;
        }

        s.max = value;

        cumulative += count;

        if (cumulative >= p50_t && s.p50 == 0) s.p50 = value;
        if (cumulative >= p95_t && s.p95 == 0) s.p95 = value;
        if (cumulative >= p99_t && s.p99 == 0) s.p99 = value;
        if (cumulative >= p999_t && s.p999 == 0) s.p999 = value;
    }

    return s;
}