#include "market_socket.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <cstring>
#include <chrono>
#include <thread>

bool MarketDataSocket::connect_to(const std::string& host, uint16_t port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);

    fcntl(sock, F_SETFL, O_NONBLOCK);

    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    int buf = 4 * 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    int ret = connect(sock, (sockaddr*)&addr, sizeof(addr));

    if (ret < 0 && errno != EINPROGRESS) {
        close(sock);
        return false;
    }

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