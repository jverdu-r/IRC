#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <map>
#include <set>

enum CommandType {
    CMD_PASS,
    CMD_UNKNOWN
};

class CommandHandler {
public:
    CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames, std::set<int>& authenticated_clients);
    void handleCommand(int client_fd, const std::string& command);
private:
    std::string server_password;
    std::map<int, std::string>& nicknames;
    std::set<int>& authenticated_clients;
    std::map<std::string, CommandType> commandMap;
};

#endif