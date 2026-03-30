#include <gtest/gtest.h>
#include "parser.h"
#include "cache.h"
#include "protocol.h"
#include <cstring>

TEST(ParserTest, ValidTradeMessage) {
    Cache cache(1000);
    Parser parser(cache);

    Header h{};
    h.type = MsgType::TRADE;
    h.seq = 1;
    h.timestamp = 123456;
    h.symbol = 1;

    Trade t{};
    t.price = 100.5;
    t.qty = 10;

    char buffer[sizeof(Header) + sizeof(Trade)];
    std::memcpy(buffer, &h, sizeof(h));
    std::memcpy(buffer + sizeof(h), &t, sizeof(t));

    parser.feed(buffer, sizeof(buffer));

    auto state = cache.get(1);

    ASSERT_DOUBLE_EQ(state.last_price, 100.5);
    ASSERT_EQ(state.last_qty, 10);
}

TEST(ParserTest, ValidQuoteMessage) {
    Cache cache(1000);
    Parser parser(cache);

    // Create QUOTE message
    Header h{};
    h.type = MsgType::QUOTE;
    h.seq = 2;
    h.timestamp = 123456;
    h.symbol = 2;

    Quote q{};
    q.bid_price = 100.0;
    q.ask_price = 101.0;
    q.bid_qty = 10;
    q.ask_qty = 20;

    char buffer[sizeof(Header) + sizeof(Quote)];
    std::memcpy(buffer, &h, sizeof(h));
    std::memcpy(buffer + sizeof(h), &q, sizeof(q));

    parser.feed(buffer, sizeof(buffer));

    auto state = cache.get(2);

    ASSERT_DOUBLE_EQ(state.best_bid, 100.0);
    ASSERT_DOUBLE_EQ(state.best_ask, 101.0);
    ASSERT_EQ(state.bid_qty, 10);
    ASSERT_EQ(state.ask_qty, 20);
}

TEST(ParserTest, FragmentedMessage) {
    Cache cache(1000);
    Parser parser(cache);

    Header h{};
    h.type = MsgType::TRADE;
    h.seq = 3;
    h.timestamp = 123456;
    h.symbol = 3;

    Trade t{};
    t.price = 200.25;
    t.qty = 5;

    char buffer[sizeof(Header) + sizeof(Trade)];
    std::memcpy(buffer, &h, sizeof(h));
    std::memcpy(buffer + sizeof(h), &t, sizeof(t));

    parser.feed(buffer, sizeof(Header));
    parser.feed(buffer + sizeof(Header), sizeof(Trade));

    auto state = cache.get(3);

    ASSERT_DOUBLE_EQ(state.last_price, 200.25);
    ASSERT_EQ(state.last_qty, 5);
}