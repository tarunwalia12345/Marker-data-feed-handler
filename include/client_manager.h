#pragma once
#include <vector>
#include <cstdint>

struct Client {
    int fd;
    std::vector<char> buffer;
};

class ClientManager {
public:
    void add(int fd);

    void remove(int fd);

    std::vector<Client> &get_all();

private:
    std::vector<Client> clients;
};
