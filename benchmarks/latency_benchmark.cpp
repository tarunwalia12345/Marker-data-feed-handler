#include <iostream>
#include <chrono>
#include <thread>
#include "../include/latency_tracker.h"

int main() {
    LatencyTracker tracker;

    for (int i = 0; i < 100000; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        std::this_thread::sleep_for(std::chrono::nanoseconds(100));

        auto end = std::chrono::high_resolution_clock::now();

        uint64_t latency =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        tracker.record(latency);
    }

    auto stats = tracker.get_stats();

    std::cout << "Mean: " << stats.mean << "\n";
    std::cout << "p99: " << stats.p99 << "\n";

    return 0;
}