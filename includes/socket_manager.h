/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket_manager.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:28 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/08 17:46:55 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

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
#include "event_handler.h"

#define MAX_EVENTS 10

class SocketManager
{
public:
    SocketManager(int port, const std::string& password);
    ~SocketManager();
	
    void								run();
    void								acceptConnection();
    void								broadcastMessage(const std::string& message, int sender_fd, const std::string& channelName);
    void								sendMessageToClient(int client_fd, const std::string& message);
    int									getEpollFd();
	std::string							getNickname(int client_fd) const;
	const std::map<int, std::string>&	getNicknames() const;

private:
    int							server_fd;
    int							epoll_fd;
    std::string					server_password;
    std::set<int>				authenticated_clients;
    std::map<int, sockaddr_in>	client_addresses;
    std::map<int, std::string>	nicknames;
    std::map<int, std::string>	usernames;
    std::map<int, std::string>	partial_messages;
    UserManager					user_manager;
    EventHandler				event_handler;
    CommandHandler				command_handler;

};
