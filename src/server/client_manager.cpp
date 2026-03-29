#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

class ClientManager {
    std::vector<int> clients;

public:
    void add(int fd) {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        clients.push_back(fd);
    }

    void broadcast(const char* data, size_t len) {
        for (auto it = clients.begin(); it != clients.end();) {
            ssize_t n = send(*it, data, len, MSG_DONTWAIT);

            if (n <= 0) {
                close(*it);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }
};