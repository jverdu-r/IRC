/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_handler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:00:56 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/21 16:42:11 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/command_handler.h"
#include "../includes/socket_manager.h"
#include "../includes/channel_manager.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

std::map<std::string, Channel> channels;
std::map<int, std::string> nicknames; // Definición de nicknames
std::set<int> authenticated_clients; // Definición de authenticated_clients

CommandHandler::CommandHandler(const std::string& server_password,
     std::map<int, std::string>& nicknames,
     std::set<int>& authenticated_clients,
     UserManager& user_manager,
     SocketManager& socket_manager)
    : server_password(server_password),
    nicknames(nicknames), authenticated_clients(authenticated_clients),
    user_manager(user_manager),
    socket_manager(socket_manager)
    {
    commandMap["/PASS"] = CMD_PASS;
    commandMap["/NICK"] = CMD_NICK;
    commandMap["/USER"] = CMD_USER;
    commandMap["/JOIN"] = CMD_JOIN;
    commandMap["/PART"] = CMD_PART;
    commandMap["/NAMES"] = CMD_NAMES;
    commandMap["/LIST"] = CMD_LIST;
    commandMap["/KICK"] = CMD_KICK;
    std::cout << "Contenido de commandMap:" << std::endl;
    for (std::map<std::string, CommandType>::const_iterator it = commandMap.begin(); it != commandMap.end(); ++it) {
        std::cout << "Comando: " << it->first << ", Tipo: " << it->second << std::endl;
    }
}

void CommandHandler::handleCommand(int client_fd, const std::string& command)
{
    if (command.empty() || command[0] != '/')
    {
        socket_manager.sendMessageToClient(client_fd, "Error: Los comandos deben comenzar con '/'.\n");
        return;
    }

    size_t spacePos = command.find(' ');
    std::string cmdName = (spacePos != std::string::npos) ? command.substr(0, spacePos) : command; // Modificado
    std::string cmdArgs = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";

    std::cout << "cmdName: " << cmdName << std::endl;

    CommandType cmdType = CMD_UNKNOWN;
    if (commandMap.find(cmdName) != commandMap.end())
    {
        cmdType = commandMap[cmdName];
    }

    switch (cmdType)
    {
        case CMD_PASS:
        {
            handlePassCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_NICK:
        {
            handleNickCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_USER:
        {
            handleUserCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_JOIN:
        {
            joinChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_PART:
        {
            partChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_NAMES:
        {
            listUsersInChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_LIST:
        {
            listChannels(client_fd);
            break;
        }
        case CMD_KICK:
        {
            handleKickCommand(client_fd, cmdArgs);
            break;
        }
        default:
            break;
    }
}

#include "authentication_commands.cpp"
#include "channel_commands.cpp"

