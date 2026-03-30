#include "client_manager.h"
#include <algorithm>
#include <unistd.h>

void ClientManager::add(int fd) {
    clients.push_back(Client{fd, {}});
}

void ClientManager::remove(int fd) {
    clients.erase(
        std::remove_if(clients.begin(), clients.end(),
                       [fd](const Client &c) {
                           if (c.fd == fd) {
                               close(c.fd);
                               return true;
                           }
                           return false;
                       }),
        clients.end()
    );
}

std::vector<Client> &ClientManager::get_all() {
    return clients;
}
