#include "exchange_simulator.h"
#include "protocol.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <thread>
#include <cstdlib>

ExchangeSimulator::ExchangeSimulator(uint16_t p, size_t n)
    : port(p), num_symbols(n) {}

void ExchangeSimulator::start() {
    std::cout << "🚀 Server started on port " << port << "\n";
}

void ExchangeSimulator::run() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);


    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 128);

    std::vector<int> clients;
    uint32_t seq = 0;

    while (true) {

        while (true) {
            int client = accept(server_fd, nullptr, nullptr);
            if (client < 0) break;

            std::cout << "Client connected\n";

            int cflags = fcntl(client, F_GETFL, 0);
            fcntl(client, F_SETFL, cflags | O_NONBLOCK);

            clients.push_back(client);
        }

        std::cout << "Sending data...\n";


        for (size_t i = 0; i < num_symbols; i++) {
            Header h{};
            h.seq = seq++;
            h.symbol = i;
            h.timestamp=std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            char buf[sizeof(Header) + sizeof(Quote)];

            if (rand() % 100 < 70) {
                h.type = MsgType::QUOTE;

                Quote q{};
                double mid = 1000 + i + (rand() % 100);

                double spread = mid * 0.001;

                q.bid_price = mid - spread;
                q.ask_price = mid + spread;
                q.bid_qty = 100;
                q.ask_qty = 100;

                memcpy(buf, &h, sizeof(Header));
                memcpy(buf + sizeof(Header), &q, sizeof(q));

                for (auto it = clients.begin(); it != clients.end();) {
                    ssize_t n = send(*it, buf, sizeof(Header) + sizeof(q), MSG_DONTWAIT);

                    if (n <= 0) {
                        close(*it);
                        it = clients.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            else {
                h.type = MsgType::TRADE;

                Trade t{};
                t.price = 1000 + i + (rand() % 100);
                t.qty = 100;

                memcpy(buf, &h, sizeof(Header));
                memcpy(buf + sizeof(Header), &t, sizeof(t));

                for (auto it = clients.begin(); it != clients.end();) {
                    ssize_t n = send(*it, buf, sizeof(Header) + sizeof(t), MSG_DONTWAIT);

                    if (n <= 0) {
                        close(*it);
                        it = clients.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ExchangeSimulator::set_tick_rate(uint32_t r) {
    tick_rate = r;
}