/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket_manager.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:28 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/20 17:55:30 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
#include "event_handler.h"

#define MAX_EVENTS 10

class SocketManager {
public:
SocketManager(int port, const std::string& password, std::map<int, std::string>& nicknames, std::set<int>& authenticated_clients, UserManager& user_manager, Authentication& authentication); // Update constructor declaration
    ~SocketManager();
    void run();
    void acceptConnection();
    void broadcastMessage(const std::string& message, int sender_fd, const std::string& channelName);
    void sendMessageToClient(int client_fd, const std::string& message);
    int getEpollFd() const { return epoll_fd; } // Método público para obtener epoll_fd

private:
    int server_fd;
    int epoll_fd;
    int port; // Added port member
    std::string password; // Added password member
    std::string server_password;
    std::map<int, sockaddr_in> client_addresses;
    std::map<int, std::string> nicknames;
    std::set<int> authenticated_clients;
    std::map<int, std::string> usernames;
    std::map<int, std::string> partial_messages;
    CommandHandler command_handler;
    UserManager user_manager;
    EventHandler event_handler;
};

#endif