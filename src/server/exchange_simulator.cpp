#include "exchange_simulator.h"
#include "protocol.h"
#include "../src/server/tick_generator.cpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>

static uint32_t checksum(const char* data, size_t len) {
    uint32_t x = 0;
    for (size_t i = 0; i < len; i++) x ^= data[i];
    return x;
}

ExchangeSimulator::ExchangeSimulator(uint16_t p, size_t n)
    : port(p), num_symbols(n) {}

void ExchangeSimulator::start() {
    std::cout << "🚀 Server started on port " << port << "\n";
}

void ExchangeSimulator::run() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1024);

    int epfd = epoll_create1(0);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    std::vector<int> clients;
    TickGenerator generator(num_symbols);

    uint32_t seq = 0;

    std::vector<char> batch;
    batch.reserve(64 * 1024);

    while (true) {

        epoll_event events[64];
        int n = epoll_wait(epfd, events, 64, 0);

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                while (true) {
                    int client = accept(server_fd, nullptr, nullptr);
                    if (client < 0) break;

                    fcntl(client, F_SETFL, O_NONBLOCK);
                    clients.push_back(client);
                }
            }
        }

        batch.clear();

        for (size_t i = 0; i < num_symbols; i++) {

            Header h{};
            h.seq = seq++;
            h.symbol = i;
            h.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

            char msg[128];
            size_t msg_len = 0;

            if (rand() % 100 < 70) {
                h.type = MsgType::QUOTE;

                Quote q{};
                double mid = generator.next(i);
                double spread = mid * (0.0005 + (rand() % 150) / 100000.0);

                q.bid_price = mid - spread;
                q.ask_price = mid + spread;
                q.bid_qty = 100;
                q.ask_qty = 100;

                memcpy(msg, &h, sizeof(h));
                memcpy(msg + sizeof(h), &q, sizeof(q));

                msg_len = sizeof(h) + sizeof(q);
            } else {
                h.type = MsgType::TRADE;

                Trade t{};
                t.price = generator.next(i);
                t.qty = 100;

                memcpy(msg, &h, sizeof(h));
                memcpy(msg + sizeof(h), &t, sizeof(t));

                msg_len = sizeof(h) + sizeof(t);
            }

            uint32_t cs = checksum(msg, msg_len);
            memcpy(msg + msg_len, &cs, 4);
            msg_len += 4;

            if (batch.size() + msg_len < 64 * 1024) {
                batch.insert(batch.end(), msg, msg + msg_len);
            }
        }

        for (auto it = clients.begin(); it != clients.end();) {
            ssize_t n = send(*it, batch.data(), batch.size(), MSG_DONTWAIT);

            if (n < 0 && errno != EAGAIN) {
                close(*it);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }

        usleep(100);
    }
}

void ExchangeSimulator::set_tick_rate(uint32_t r) {
    tick_rate = r;
}