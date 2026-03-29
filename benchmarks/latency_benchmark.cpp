#include <iostream>
#include <chrono>
#include <thread>   // 🔥 for sleep
#include "latency_tracker.h"

int main() {
    LatencyTracker tracker;

    for (int i = 0; i < 100000; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // simulate work (clean, no warnings)
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));

        auto end = std::chrono::high_resolution_clock::now();

        uint64_t latency =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        tracker.record(latency);
    }

    auto stats = tracker.get_stats();

    std::cout << "===== LATENCY STATS =====\n";
    std::cout << "Samples: " << stats.sample_count << "\n";
    std::cout << "Mean:    " << stats.mean << " ns\n";
    std::cout << "Min:     " << stats.min << " ns\n";
    std::cout << "Max:     " << stats.max << " ns\n";
    std::cout << "p50:     " << stats.p50 << " ns\n";
    std::cout << "p95:     " << stats.p95 << " ns\n";
    std::cout << "p99:     " << stats.p99 << " ns\n";
    std::cout << "p999:    " << stats.p999 << " ns\n";

    return 0;
}