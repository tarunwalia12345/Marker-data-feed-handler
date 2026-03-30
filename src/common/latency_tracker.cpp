#include "latency_tracker.h"

size_t LatencyTracker::bucket_index(uint64_t ns) const {
    size_t bucket = 0;

    uint64_t us = ns / 1000;

    while (us > 1 && bucket + 1 < BUCKETS) {
        us >>= 1;
        bucket++;
    }

    return bucket;
}

void LatencyTracker::record(uint64_t latency_ns) {
    total_samples.fetch_add(1, std::memory_order_relaxed);
    sum.fetch_add(latency_ns, std::memory_order_relaxed);

    size_t b = bucket_index(latency_ns);
    histogram[b].fetch_add(1, std::memory_order_relaxed);

    size_t i = idx.fetch_add(1, std::memory_order_relaxed) % MAX_SAMPLES;
    samples[i] = latency_ns;

    uint64_t prev_min = min_latency.load(std::memory_order_relaxed);
    while (latency_ns < prev_min &&
           !min_latency.compare_exchange_weak(prev_min, latency_ns,
                                              std::memory_order_relaxed));

    uint64_t prev_max = max_latency.load(std::memory_order_relaxed);
    while (latency_ns > prev_max &&
           !max_latency.compare_exchange_weak(prev_max, latency_ns,
                                              std::memory_order_relaxed));
}

LatencyTracker::Stats LatencyTracker::get_stats() const {
    Stats s{};

    uint64_t total = total_samples.load(std::memory_order_relaxed);
    s.sample_count = total;

    if (total == 0) return s;

    s.mean = sum.load(std::memory_order_relaxed) / total;
    s.min = min_latency.load(std::memory_order_relaxed);
    s.max = max_latency.load(std::memory_order_relaxed);


    uint64_t t50 = total * 50 / 100;
    uint64_t t95 = total * 95 / 100;
    uint64_t t99 = total * 99 / 100;
    uint64_t t999 = total * 999 / 1000;

    uint64_t cumulative = 0;

    for (size_t i = 0; i < BUCKETS; i++) {
        cumulative += histogram[i].load(std::memory_order_relaxed);

        uint64_t value = (1ULL << i) * 1000; // µs → ns

        if (cumulative >= t50 && s.p50 == 0) s.p50 = value;
        if (cumulative >= t95 && s.p95 == 0) s.p95 = value;
        if (cumulative >= t99 && s.p99 == 0) s.p99 = value;
        if (cumulative >= t999 && s.p999 == 0) s.p999 = value;
    }

    return s;
}

void LatencyTracker::export_csv() const {
    std::ofstream file("latency.csv");

    for (size_t i = 0; i < MAX_SAMPLES; i++) {
        file << samples[i] << "\n";
    }
}
