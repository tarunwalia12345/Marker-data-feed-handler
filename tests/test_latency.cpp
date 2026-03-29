#include <gtest/gtest.h>
#include "../src/common/latency_tracker.cpp"

TEST(LatencyTest, BasicStats) {
    LatencyTracker tracker;

    for (int i = 1; i <= 1000; i++) {
        tracker.record(i);
    }

    auto stats = tracker.get_stats();

    ASSERT_GT(stats.mean, 0);
    ASSERT_GT(stats.p99, 0);
}