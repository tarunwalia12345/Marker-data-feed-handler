#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <chrono>
#include <atomic>

#include "cache.h"
#include "market_socket.h"
#include "parser.h"
#include "latency_tracker.h"

// ================= SHARED STATS =================
struct Stats {
    std::atomic<uint64_t> messages{0};
    std::atomic<uint64_t> updates{0};
    std::atomic<uint64_t> seq_gaps{0};
};

// ✅ DEFINE ONLY HERE
Stats stats;
LatencyTracker latency;

// no header
void render(Cache&, uint64_t);

int main() {
    MarketDataSocket socket;

    while (!socket.connect_to("127.0.0.1", 9876)) {
        std::cout << "Retrying...\n";
        sleep(1);
    }

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
    auto last_render = std::chrono::steady_clock::now();

    while (true) {
        epoll_event events[10];
        int n = epoll_wait(epfd, events, 10, 100);

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == sock) {

                if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                    goto shutdown;
                }

                while (true) {
                    char buf[4096];

                    ssize_t len = recv(sock, buf, sizeof(buf), 0);

                    if (len > 0) {
                        total_msgs++;

                        stats.messages.fetch_add(1, std::memory_order_relaxed);
                        stats.updates.fetch_add(1, std::memory_order_relaxed);

                        auto t1 = std::chrono::high_resolution_clock::now();

                        parser.feed(buf, len);

                        auto t2 = std::chrono::high_resolution_clock::now();

                        latency.record(
                            std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
                        );
                    }
                    else if (len == 0) {
                        goto shutdown;
                    }
                    else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        if (errno == EINTR) continue;
                        perror("recv");
                        goto shutdown;
                    }
                }
            }
        }

        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_render).count() > 500) {
            render(cache, total_msgs);
            last_render = now;
        }

        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'q') break;

            if (c == 'r') {
                stats.messages.store(0, std::memory_order_relaxed);
                stats.updates.store(0, std::memory_order_relaxed);
                stats.seq_gaps.store(0, std::memory_order_relaxed);
            }
        }
    }

shutdown:
    socket.disconnect();
    close(epfd);

    std::cout << "Client exited cleanly\n";
    return 0;
}