/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user_manager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:15 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 18:16:38 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/user_manager.h"
#include <iostream>

UserManager::UserManager(std::map<int, std::string>& usernames) : usernames(usernames) {}

void UserManager::setUserName(int client_fd, const std::string& username)
{
	usernames[client_fd] = username;
}

std::string UserManager::getUserName(int client_fd)
{
	if (usernames.find(client_fd) != usernames.end())
	{
		return usernames[client_fd];
	}
	return "";
}

std::string UserManager::getUserChannel(int client_fd) const {
    if (user_channels.find(client_fd) != user_channels.end()) {
        return user_channels.at(client_fd);
    }
    return ""; // O algún valor predeterminado si el usuario no tiene un canal
}

void UserManager::setUserChannel(int client_fd, const std::string& channel){
    user_channels[client_fd] = channel;
    std::cout << "Usuario " << client_fd << " se ha unido al canal: " << channel << std::endl;
}