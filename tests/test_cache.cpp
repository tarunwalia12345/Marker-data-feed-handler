#include <gtest/gtest.h>
#include "cache.h"
#include <thread>

TEST(CacheTest, SingleThreadReadWrite) {
    Cache cache(100);

    cache.update_quote(1, 100.5, 10, 101.5, 20);
    cache.update_trade(1, 101.0, 15);

    auto snap = cache.get(1);

    ASSERT_DOUBLE_EQ(snap.best_bid, 100.5);
    ASSERT_DOUBLE_EQ(snap.best_ask, 101.5);
    ASSERT_EQ(snap.bid_qty, 10);
    ASSERT_EQ(snap.ask_qty, 20);

    ASSERT_DOUBLE_EQ(snap.last_price, 101.0);
    ASSERT_EQ(snap.last_qty, 15);
}

TEST(CacheTest, MultiThreadedReadWrite) {
    Cache cache(100);

    std::thread writer([&]() {
        for (int i = 0; i < 100000; i++) {
            cache.update_trade(1, 100.0 + i, i);
        }
    });

    std::thread reader([&]() {
        for (int i = 0; i < 100000; i++) {
            auto snap = cache.get(1);
            (void)snap;
        }
    });

    writer.join();
    reader.join();

    SUCCEED();
}