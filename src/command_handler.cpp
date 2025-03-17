#include "../includes/command_handler.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

CommandHandler::CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames, std::set<int>& authenticated_clients)
    : server_password(server_password), nicknames(nicknames), authenticated_clients(authenticated_clients) {
    commandMap["PASS"] = CMD_PASS;
}

void CommandHandler::handleCommand(int client_fd, const std::string& command) {
    size_t spacePos = command.find(' ');
    std::string cmdName = command.substr(0, spacePos);
    std::string cmdArgs = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";

    CommandType cmdType = CMD_UNKNOWN;
    if (commandMap.find(cmdName) != commandMap.end()) {
        cmdType = commandMap[cmdName];
    }

    switch (cmdType) {
        case CMD_PASS: {
            std::string client_password = cmdArgs.substr(0, cmdArgs.find('\n'));
            if (client_password == server_password) {
                send(client_fd, "Contraseña correcta. Bienvenido al servidor IRC.\n", 50, 0);
                nicknames[client_fd] = "Invitado";
                authenticated_clients.insert(client_fd);
            } else {
                send(client_fd, "Contraseña incorrecta. Inténtalo de nuevo.\n", 38, 0);
                close(client_fd);
            }
            break;
        }
        case CMD_UNKNOWN:
        default:
            send(client_fd, "Comando desconocido.\n", 20, 0);
            break;
    }
}