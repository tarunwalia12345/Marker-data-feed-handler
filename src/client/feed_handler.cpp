#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "cache.h"
#include "market_socket.h"
#include "parser.h"
#include "latency_tracker.h"

struct Stats {
    std::atomic<uint64_t> messages{0};
    std::atomic<uint64_t> updates{0};
    std::atomic<uint64_t> seq_gaps{0};
};

Stats stats;
LatencyTracker latency;

void render(Cache &, uint64_t);

int main() {
    MarketDataSocket socket;

    int retry = 0;

    auto last_msg = std::chrono::steady_clock::now();

    std::atomic<bool> running{false};
    std::thread ui_thread;
    std::thread parser_thread;
    std::queue<std::vector<char> > q;
    std::mutex m;
    std::condition_variable cv;

    while (true) {
        if (!socket.connect_to("127.0.0.1", 9877)) {
            std::cout << "Reconnect attempt...\n";

            std::this_thread::sleep_for(
                std::chrono::milliseconds(100 * (1 << std::min(retry, 5)))
            );

            retry++;
            continue;
        }

        retry = 0;
        std::cout << "Connected to server\n";

        std::vector<uint16_t> ids(100);
        for (int i = 0; i < 100; i++) ids[i] = static_cast<uint16_t>(i);
        socket.send_subscription(ids);

        int sock = socket.get_fd();

        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

        int epfd = epoll_create1(0);

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = sock;

        epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);

        Cache cache(100);
        Parser parser(cache);

        uint64_t total_msgs = 0;

        running = true;

        ui_thread = std::thread([&] {
            while (running) {
                render(cache, total_msgs);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });

        parser_thread = std::thread([&] {
            while (running) {
                std::vector<char> data;

                {
                    std::unique_lock<std::mutex> lock(m);
                    cv.wait(lock, [&] { return !q.empty() || !running; });

                    if (!running) return;

                    data = std::move(q.front());
                    q.pop();
                }

                auto t1 = std::chrono::high_resolution_clock::now();

                parser.feed(data.data(), data.size());

                auto t2 = std::chrono::high_resolution_clock::now();

                latency.record(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
                );
            }
        });

        while (true) {
            epoll_event events[10];
            int n = epoll_wait(epfd, events, 10, 100);

            for (int i = 0; i < n; i++) {
                if (events[i].data.fd == sock) {
                    if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                        std::cout << "Socket error, reconnecting...\n";
                        goto reconnect;
                    }

                    while (true) {
                        char buf[4096];

                        ssize_t len = recv(sock, buf, sizeof(buf), 0);

                        if (len > 0) {
                            last_msg = std::chrono::steady_clock::now();
                            total_msgs++;

                            stats.messages.fetch_add(1);
                            stats.updates.fetch_add(1);

                            {
                                std::lock_guard<std::mutex> lock(m);

                                if (q.size() > 10000) q.pop(); // bounded

                                q.emplace(buf, buf + len);
                            }

                            cv.notify_one();
                        } else if (len == 0) {
                            std::cout << "Server closed connection\n";
                            goto reconnect;
                        } else {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            if (errno == EINTR) continue;

                            perror("recv");
                            goto reconnect;
                        }
                    }
                }
            }

            if (std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - last_msg).count() > 5) {
                std::cout << "Heartbeat timeout\n";
                goto reconnect;
            }

            char c;
            if (read(STDIN_FILENO, &c, 1) > 0) {
                if (c == 'q') goto shutdown;

                if (c == 'r') {
                    stats.messages.store(0);
                    stats.updates.store(0);
                    stats.seq_gaps.store(0);
                }
            }
        }

    reconnect:
        socket.disconnect();
        close(epfd);

        running = false;
        cv.notify_all();

        if (ui_thread.joinable()) ui_thread.join();
        if (parser_thread.joinable()) parser_thread.join();

        std::cout << "Reconnecting...\n";
        continue;
    }

shutdown:
    running = false;
    cv.notify_all();

    if (ui_thread.joinable()) ui_thread.join();
    if (parser_thread.joinable()) parser_thread.join();

    socket.disconnect();

    std::cout << "Client exited\n";
    return 0;
}
