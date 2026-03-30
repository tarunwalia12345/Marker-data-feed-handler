#pragma once
#include <string>
#include <cstdint>
#include <vector>

class MarketDataSocket {
    int sock{-1};

public:
    bool connect_to(const std::string &host, uint16_t port);

    ssize_t receive(void *buffer, size_t len);

    bool is_connected() const;

    bool send_subscription(const std::vector<uint16_t> &symbol_ids);

    void disconnect();

    int get_fd() const;
};
