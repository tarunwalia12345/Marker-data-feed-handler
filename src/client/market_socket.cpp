#include "market_socket.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

bool MarketDataSocket::connect_to(const std::string& host, uint16_t port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0)
        return false;

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0)
        return false;

    return true;
}

ssize_t MarketDataSocket::receive(void* buffer, size_t len) {
    return recv(sock, buffer, len, 0);
}

bool MarketDataSocket::is_connected() const {
    return sock > 0;
}

void MarketDataSocket::disconnect() {
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
}
int MarketDataSocket::get_fd() const {
    return sock;
}