#include "parser.h"
#include "protocol.h"
#include <cstring>
#include <iostream>

Parser::Parser(Cache& c) : cache(c) {
    buffer.reserve(1 << 20);
}

void Parser::on_data(const char* data, size_t len) {
    buffer.insert(buffer.end(), data, data + len);

    size_t offset = 0;

    while (buffer.size() - offset >= sizeof(Header)) {
        auto* h = reinterpret_cast<Header*>(buffer.data() + offset);

        size_t msg_size = sizeof(Header);

        if (h->type == MsgType::TRADE)
            msg_size += sizeof(Trade);
        else if (h->type == MsgType::QUOTE)
            msg_size += sizeof(Quote);
        else
            break;

        if (buffer.size() - offset < msg_size)
            break;

        if (h->type == MsgType::TRADE) {
            auto* t = reinterpret_cast<Trade*>(buffer.data() + offset + sizeof(Header));

            cache.update_trade(h->symbol, t->price, t->qty);

            std::cout << "TRADE update symbol: " << h->symbol
                      << " price: " << t->price << "\n";
        }
        else if (h->type == MsgType::QUOTE) {
            auto* q = reinterpret_cast<Quote*>(buffer.data() + offset + sizeof(Header));

            cache.update_quote(
                h->symbol,
                q->bid, q->bid_qty,
                q->ask, q->ask_qty
            );

            std::cout << "QUOTE update symbol: " << h->symbol
                      << " bid: " << q->bid
                      << " ask: " << q->ask << "\n";
        }

        offset += msg_size;
    }
    buffer.erase(buffer.begin(), buffer.begin() + offset);
}