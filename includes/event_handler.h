/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_handler.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:32 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 16:01:33 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <string>
#include <map>
#include <set>
#include <sys/socket.h>
#include <netinet/in.h> // Incluir netinet/in.h
#include "command_handler.h"
#include "user_manager.h"

class SocketManager; // Declaración adelantada

class EventHandler {
public:
    EventHandler(SocketManager& socket_manager, CommandHandler& command_handler, UserManager& user_manager, std::map<int, std::string>& partial_messages, std::map<int, sockaddr_in>& client_addresses, std::set<int>& authenticated_clients);
    void handleClientEvent(int client_fd);

private:
    SocketManager& socket_manager;
    CommandHandler& command_handler;
    UserManager& user_manager;
    std::map<int, std::string>& partial_messages;
    std::map<int, sockaddr_in>& client_addresses;
    std::set<int>& authenticated_clients;
};

#endif