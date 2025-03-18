#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <string>
#include <set>
#include "command_handler.h"
#include "user_manager.h"

#define MAX_EVENTS 10

class SocketManager {
public:
    SocketManager(int port, const std::string& password);
    ~SocketManager();
    void run();
    void acceptConnection();
    void handleClientEvent(int client_fd);
    void broadcastMessage(const std::string& message, int sender_fd);
    void sendMessageToClient(int client_fd, const std::string& message);
private:
    int server_fd;
    int epoll_fd;
    std::string server_password;
    std::map<int, sockaddr_in> client_addresses;
    std::map<int, std::string> nicknames;
    std::set<int> authenticated_clients;
    CommandHandler command_handler; // command_handler debe ir antes de user_manager
    UserManager user_manager;
    std::map<int, std::string> usernames;
    std::map<int, std::string> partial_messages;
};

#endif