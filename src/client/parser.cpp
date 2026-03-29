#include "parser.h"
#include <cstring>

Parser::Parser(Cache& c) : cache(c) {}

ParseResult Parser::parse(const char* data, size_t len) {
    if (len < sizeof(Header)) return {false};

    Header h;
    std::memcpy(&h, data, sizeof(Header));

    uint16_t symbol = h.symbol;
    if (symbol >= 1000) return {false};

    auto type = static_cast<uint16_t>(h.type);

    // 🔥 TRADE
    if (type == static_cast<uint16_t>(MsgType::TRADE)) {
        if (len < sizeof(Header) + sizeof(Trade)) return {false};

        Trade t;
        std::memcpy(&t, data + sizeof(Header), sizeof(Trade));

        cache.update_trade(symbol, t.price, t.qty);
    }
    // 🔥 QUOTE
    else if (type == static_cast<uint16_t>(MsgType::QUOTE)) {
        if (len < sizeof(Header) + sizeof(Quote)) return {false};

        Quote q;
        std::memcpy(&q, data + sizeof(Header), sizeof(Quote));

        cache.update_quote(symbol,
                           q.bid_price, q.bid_qty,
                           q.ask_price, q.ask_qty);
    }
    // 🔥 HEARTBEAT (no payload)
    else if (type == static_cast<uint16_t>(MsgType::HEARTBEAT)) {
        // do nothing
    }
    else {
        return {false}; // unknown type
    }

    return {true};
}

void Parser::feed(const char* data, size_t len) {
    // Append incoming TCP data
    buffer.insert(buffer.end(), data, data + len);

    size_t offset = 0;

    while (true) {
        size_t remaining = buffer.size() - offset;

        // Not enough for header
        if (remaining < sizeof(Header)) break;

        Header h;
        std::memcpy(&h, buffer.data() + offset, sizeof(Header));

        size_t msg_size = sizeof(Header);
        auto type = static_cast<uint16_t>(h.type);

        // Determine full message size
        if (type == static_cast<uint16_t>(MsgType::TRADE)) {
            msg_size += sizeof(Trade);
        }
        else if (type == static_cast<uint16_t>(MsgType::QUOTE)) {
            msg_size += sizeof(Quote);
        }
        else if (type == static_cast<uint16_t>(MsgType::HEARTBEAT)) {
            // no payload
        }
        else {
            // ❗ Unknown data → resync (VERY IMPORTANT)
            offset += 1;
            continue;
        }

        // Incomplete message → wait for more data
        if (remaining < msg_size) break;

        // Process complete message
        parse(buffer.data() + offset, msg_size);

        offset += msg_size;
    }

    // Remove processed bytes
    if (offset > 0) {
        buffer.erase(buffer.begin(), buffer.begin() + offset);
    }
}