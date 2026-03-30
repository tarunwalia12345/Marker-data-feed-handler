#include "exchange_simulator.h"
#include "../common/protocol.h"
#include "tick_generator.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <thread>
#include <errno.h>
#include <atomic>
#include <random>

static uint32_t checksum(const char *data, size_t len) {
    uint32_t x = 0;
    for (size_t i = 0; i < len; i++) x ^= data[i];
    return x;
}

struct Client {
    int fd;
    std::vector<char> buffer;
};

ExchangeSimulator::ExchangeSimulator(uint16_t p, size_t n)
    : port(p), num_symbols(n) {}

void ExchangeSimulator::start() {
    std::cout << "Server started on port " << port << "\n";
}

void ExchangeSimulator::run() {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int sndbuf = 4 * 1024 * 1024;
    setsockopt(server_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

    if (listen(server_fd, 1024) < 0) {
        perror("listen");
        return;
    }

    int epfd = epoll_create1(0);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    std::vector<Client> clients;
    TickGenerator generator(static_cast<int>(num_symbols));

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> type_dist(0, 99);
    std::uniform_int_distribution<int> qty_dist(50, 500);

    uint32_t seq = 0;
    std::vector<char> batch;
    batch.reserve(64 * 1024);

    auto last = std::chrono::high_resolution_clock::now();

    std::atomic<bool> running{true};

    while (running) {

        epoll_event events[64];
        int n = epoll_wait(epfd, events, 64, 0);

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {

                while (true) {
                    int client_fd = accept(server_fd, nullptr, nullptr);

                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("accept");
                        break;
                    }

                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    clients.push_back({client_fd, {}});

                    epoll_event cev{};
                    cev.events = EPOLLIN | EPOLLRDHUP;
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);
                }
            }
        }

        for (auto it = clients.begin(); it != clients.end();) {

            Client &c = *it;

            if (!c.buffer.empty()) {

                ssize_t sent = send(c.fd, c.buffer.data(), c.buffer.size(), MSG_DONTWAIT);

                if (sent > 0) {
                    c.buffer.erase(c.buffer.begin(), c.buffer.begin() + sent);
                    ++it;
                } else if (sent < 0 && errno != EAGAIN) {
                    close(c.fd);
                    it = clients.erase(it);
                } else {
                    ++it;
                }

            } else {
                ++it;
            }
        }

        batch.clear();

        for (size_t i = 0; i < num_symbols; i++) {

            Header h{};
            h.seq = seq++;
            h.symbol = static_cast<uint16_t>(i);
            h.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

            char msg[128];
            size_t msg_len = 0;

            double mid = generator.next((int)i);
            double spread = mid * 0.0005;

            double bid = mid - spread;
            double ask = mid + spread;

            // safety
            if (bid <= 0) bid = 1.0;
            if (ask <= bid) ask = bid + 0.01;

            bool send_quote = (type_dist(rng) < 70);

            if (send_quote) {

                h.type = MsgType::QUOTE;

                Quote q{};
                q.bid_price = bid;
                q.ask_price = ask;
                q.bid_qty = qty_dist(rng);
                q.ask_qty = qty_dist(rng);

                memcpy(msg, &h, sizeof(h));
                memcpy(msg + sizeof(h), &q, sizeof(q));

                msg_len = sizeof(h) + sizeof(q);

            } else {

                h.type = MsgType::TRADE;

                Trade t{};

                double trade_price = std::min(std::max(mid, bid), ask);

                t.price = trade_price;
                t.qty = qty_dist(rng);

                memcpy(msg, &h, sizeof(h));
                memcpy(msg + sizeof(h), &t, sizeof(t));

                msg_len = sizeof(h) + sizeof(t);
            }

            // ===== CHECKSUM =====
            uint32_t cs = checksum(msg, msg_len);
            memcpy(msg + msg_len, &cs, 4);
            msg_len += 4;

            if (batch.size() + msg_len < 64 * 1024) {
                batch.insert(batch.end(), msg, msg + msg_len);
            }
        }

        if (!clients.empty()) {

            size_t start = seq % clients.size();

            for (size_t i = 0; i < clients.size(); i++) {

                Client &c = clients[(start + i) % clients.size()];

                ssize_t sent = send(c.fd, batch.data(), batch.size(), MSG_DONTWAIT);

                if (sent < 0) {
                    if (errno == EAGAIN) {
                        c.buffer.insert(c.buffer.end(), batch.begin(), batch.end());
                    } else {
                        close(c.fd);
                    }
                }
            }
        }

        auto now = std::chrono::high_resolution_clock::now();

        double elapsed = std::chrono::duration<double>(now - last).count();
        double expected = static_cast<double>(num_symbols) / tick_rate;

        if (elapsed < expected) {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(expected - elapsed)
            );
        }

        last = std::chrono::high_resolution_clock::now();
    }
}

void ExchangeSimulator::set_tick_rate(uint32_t r) {
    tick_rate = r;
}