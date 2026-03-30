#include "parser.h"
#include <cstring>
#include <iostream>
#include "../common/protocol.h"
static uint32_t checksum(const char *data, size_t len) {
    uint32_t x = 0;
    for (size_t i = 0; i < len; i++) x ^= data[i];
    return x;
}

static uint32_t last_seq = 0;

Parser::Parser(Cache &c) : cache(c) {
}

void Parser::feed(const char *data, size_t len) {
    buffer.insert(buffer.end(), data, data + len);

    size_t offset = 0;

    while (true) {
        if (buffer.size() - offset < sizeof(Header)) break;

        const Header *h = reinterpret_cast<const Header *>(buffer.data() + offset);

        size_t msg_size = sizeof(Header);

        if (h->type != MsgType::TRADE &&
            h->type != MsgType::QUOTE &&
            h->type != MsgType::HEARTBEAT) {
            offset += sizeof(uint16_t);
            continue;
        }

        if (h->type == MsgType::TRADE) msg_size += sizeof(Trade);
        else if (h->type == MsgType::QUOTE) msg_size += sizeof(Quote);

        msg_size += 4;


        if (msg_size > 1024) {
            offset += sizeof(uint16_t);
            continue;
        }

        if (buffer.size() - offset < msg_size) break;

        uint32_t expected_cs;
        memcpy(&expected_cs, buffer.data() + offset + msg_size - 4, 4);

        uint32_t actual_cs = checksum(buffer.data() + offset, msg_size - 4);

        if (expected_cs != actual_cs) {
            offset += sizeof(uint16_t);
            continue;
        }

        if (last_seq && h->seq != last_seq + 1) {
            std::cout << "SEQ GAP: " << last_seq << " -> " << h->seq << "\n";
        }

        last_seq = h->seq;

        if (h->type == MsgType::TRADE) {
            const Trade *t = reinterpret_cast<const Trade *>(buffer.data() + offset + sizeof(Header));

            cache.update_trade(h->symbol, t->price, t->qty);
        } else if (h->type == MsgType::QUOTE) {
            const Quote *q = reinterpret_cast<const Quote *>(buffer.data() + offset + sizeof(Header));

            cache.update_quote(h->symbol,
                               q->bid_price, q->bid_qty,
                               q->ask_price, q->ask_qty);
        } else if (h->type == MsgType::HEARTBEAT) {
        }

        offset += msg_size;
    }

    if (offset) buffer.erase(buffer.begin(), buffer.begin() + offset);
}
