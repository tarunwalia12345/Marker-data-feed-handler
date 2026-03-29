#pragma once
#include <cstdint>

#pragma pack(push, 1)

enum class MsgType : uint16_t {
    TRADE = 0x01,
    QUOTE = 0x02,
    HEARTBEAT = 0x03
};

struct Header {
    MsgType type;
    uint32_t seq;
    uint64_t timestamp;
    uint16_t symbol;
};

struct Trade {
    double price;
    uint32_t qty;
};

struct Quote {
    double bid_price;
    uint32_t bid_qty;
    double ask_price;
    uint32_t ask_qty;
};

#pragma pack(pop)