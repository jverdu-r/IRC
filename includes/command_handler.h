/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_handler.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:35 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 16:56:08 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <map>
#include <set>
#include "user_manager.h"
#include "channel_manager.h"

// Declaración adelantada de SocketManager
class SocketManager;

enum CommandType
{
    CMD_PASS,
    CMD_NICK,
    CMD_USER,
    CMD_JOIN,
    CMD_PART,
    CMD_NAMES,
    CMD_LIST,
    CMD_KICK,
    CMD_UNKNOWN
};

extern std::map<std::string, Channel> channels;

class CommandHandler
{
    public:
        CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames, std::set<int>& authenticated_clients, UserManager& user_manager, SocketManager& socket_manager); // Añadir SocketManager al constructor
        void handleCommand(int client_fd, const std::string& command);
        void kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName);
        void listChannels(int client_fd);
        void listUsersInChannel(int client_fd, const std::string& channelName);
        void partChannel(int client_fd, const std::string& channelName);
        void joinChannel(int client_fd, const std::string& channelName);
        const std::map<std::string, Channel>& getChannels() const;

    private:
        std::string server_password;
        std::map<int, std::string>& nicknames;
        std::set<int>& authenticated_clients;
        std::map<std::string, CommandType> commandMap;
        UserManager& user_manager;
        SocketManager& socket_manager;
        
        
};

#endif