/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_handler.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:32 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/08 10:02:40 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <set>
#include <sys/socket.h>
#include <netinet/in.h> // Incluir netinet/in.h
#include "command_handler.h"
#include "user_manager.h"

class SocketManager; // Declaración adelantada

class EventHandler
{
public:
    EventHandler(SocketManager& socket_manager, CommandHandler& command_handler, UserManager& user_manager,
		std::map<int, std::string>& partial_messages, std::map<int, sockaddr_in>& client_addresses,
		std::set<int>& authenticated_clients);
	~EventHandler();
	
	void handleClientEvent(int client_fd);

private:
    std::set<int>&				authenticated_clients;
    std::map<int, std::string>&	partial_messages;
    std::map<int, sockaddr_in>&	client_addresses;
    SocketManager& 				socket_manager;
    CommandHandler&				command_handler;
    UserManager&				user_manager;

    void handleClientDisconnect(int client_fd, int bytes_received);
    void processReceivedData(int client_fd, const std::string& received_data);
    void processLine(int client_fd, const std::string& line);
    void assignDefaultUsername(int client_fd);
};
