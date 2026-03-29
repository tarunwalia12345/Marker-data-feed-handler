#pragma once
#include <cstdint>

#pragma pack(push, 1)

enum class MsgType : uint16_t {
    TRADE = 1,
    QUOTE = 2,
    HEARTBEAT = 3
};

struct Header {
    MsgType type;
    uint32_t seq;
    uint64_t ts;
    uint16_t symbol;
};

struct Trade {
    double price;
    uint32_t qty;
};

struct Quote {
    double bid;
    uint32_t bid_qty;
    double ask;
    uint32_t ask_qty;
};

#pragma pack(pop)