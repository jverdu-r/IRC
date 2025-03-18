#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <map>
#include <set>
#include "user_manager.h"

// Declaración adelantada de SocketManager
class SocketManager;

enum CommandType
{
    CMD_PASS,
    CMD_NICK,
    CMD_USER,
    CMD_UNKNOWN
};

class CommandHandler
{
    public:
        CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames, std::set<int>& authenticated_clients, UserManager& user_manager, SocketManager& socket_manager); // Añadir SocketManager al constructor
        void handleCommand(int client_fd, const std::string& command);
    private:
        std::string server_password;
        std::map<int, std::string>& nicknames;
        std::set<int>& authenticated_clients;
        std::map<std::string, CommandType> commandMap;
        UserManager& user_manager;
        SocketManager& socket_manager;
};

#endif