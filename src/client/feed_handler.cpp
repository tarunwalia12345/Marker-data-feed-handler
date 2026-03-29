#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <chrono>

#include "cache.h"
#include "market_socket.h"
#include "parser.h"
#include<sys/socket.h>

extern void render(Cache&);

int main() {
    MarketDataSocket socket;

    // ================= CONNECT =================
    while (!socket.connect_to("127.0.0.1", 9876)) {
        std::cout << "Retrying...\n";
        sleep(1);
    }

    std::cout << "Connected to server\n";

    int sock = socket.get_fd();

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    int epfd = epoll_create1(0);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sock;

    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);


    Cache cache(100);
    Parser parser(cache);

    std::cout << "Client started\n";

    int max_iterations = 20;
    int max_messages   = 10000;

    int iteration = 0;
    int total_msgs = 0;

    auto start_time = std::chrono::steady_clock::now();

    while (iteration < max_iterations && total_msgs < max_messages) {
        epoll_event events[10];

        int n = epoll_wait(epfd, events, 10, 1000);

        if (n <= 0) {
            std::cout << "⏳ Waiting for data...\n";
            continue;
        }

        iteration++;

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == sock) {

                while (true) {
                    char buf[4096];

                    ssize_t len = recv(sock, buf, sizeof(buf), 0);

                    if (len > 0) {
                        total_msgs++;

                        std::cout << "Received: " << len << " bytes\n";

                        parser.on_data(buf, len);
                    }
                    else if (len == 0) {
                        std::cout << "Server disconnected\n";
                        goto shutdown;
                    }
                    else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("recv error");
                            goto shutdown;
                        }
                    }
                }
            }
        }

        render(cache);
    }

shutdown:

    auto end_time = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration<double>(end_time - start_time).count();

    std::cout << "\n Client Stopped\n";
    std::cout << "Iterations: " << iteration << "\n";
    std::cout << "Messages: " << total_msgs << "\n";
    std::cout << "Time: " << seconds << " sec\n";

    if (seconds > 0)
        std::cout << "Throughput: " << (total_msgs / seconds) << " msgs/sec\n";

    socket.disconnect();
    close(epfd);

    std::cout << " Client exited cleanly\n";

    return 0;
}