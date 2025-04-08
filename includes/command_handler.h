/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_handler.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:35 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/06 20:53:10 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <set>
#include "user_manager.h"
#include "channel_manager.h"

class SocketManager; // Declaración adelantada de SocketManager
extern std::map<std::string, Channel> channels;

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
    CMD_UNKNOWN,
	CMD_PRIVMSG,
	CMD_MODE,
	CMD_INVITE,
	CMD_TOPIC,
	CMD_WHEREIS,
	CMD_WHEREAMI,
	CMD_ACTIVE,
};

class CommandHandler
{
    public:
        CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames,
			std::set<int>& authenticated_clients, UserManager& user_manager, SocketManager& socket_manager);
		~CommandHandler();
		
        void									handleCommand(int client_fd, const std::string& command);
        void									kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName);
        void									listChannels(int client_fd);
        void									listUsersInChannel(int client_fd, const std::string& channelName);
        void									partChannel(int client_fd, const std::string& channelName);
        void									joinChannel(int client_fd, const std::string& channelName);
        void									handlePassCommand(int client_fd, const std::string& cmdArgs);
        void									handleKickCommand(int client_fd, const std::string& cmdArgs);
        void									handleUserCommand(int client_fd, const std::string& cmdArgs);
        void									handleNickCommand(int client_fd, const std::string& cmdArgs);
		void									handlePrivMsgCommand(int client_fd, const std::string& cmdArgs);
		void 									handleInviteCommand(int client_fd, const std::string& cmdArgs);
		void 									handleTopicCommand(int client_fd, const std::string& cmdArgs);
		void 									handleModeCommand(int client_fd, const std::string& cmdArgs);
        void 									handleWhereIsCommand(int client_fd, const std::string& cmdArgs);
		void 									handleWhereAmICommand(int client_fd, const std::string& cmdArgs);
		void 									handleActiveCommand(int client_fd, const std::string& cmdArgs);
		const std::map<std::string, Channel>&	getChannels() const;

    private:
        std::string							server_password;
        std::set<int>&						authenticated_clients;
        std::map<int, std::string>&			nicknames;
        std::map<std::string, CommandType>	commandMap;
        UserManager&						user_manager;
        SocketManager&						socket_manager;
};
