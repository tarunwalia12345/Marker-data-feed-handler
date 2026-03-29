#pragma once
#include <string>
#include <cstdint>

class MarketDataSocket {
    int sock{-1};

public:
    bool connect_to(const std::string& host, uint16_t port);
    ssize_t receive(void* buffer, size_t len);
    bool is_connected() const;
    void disconnect();

    int get_fd() const;
};