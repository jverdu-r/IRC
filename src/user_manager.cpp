/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user_manager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:15 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/08 18:48:47 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/user_manager.h"
#include "../includes/utils.h"
#include "../includes/socket_manager.h"
#include <iostream>
#include <unistd.h>

UserManager::UserManager(std::map<int, std::string>& usernames, SocketManager& socket_manager) :
	usernames(usernames),
	socket_manager(socket_manager)
{
    this->socket_manager.sendMessageToClient(0, "Gestor de usuarios inicializado");
}

UserManager::~UserManager()
{
}

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

bool UserManager::userNameExists(const std::string& username)
{
    for (std::map<int, std::string>::const_iterator it = usernames.begin(); it != usernames.end(); ++it)
	{
        if (it->second == username)
		{
            return true;
        }
    }
    return false;
}

void UserManager::addUserChannel(int client_fd, const std::string& channel)
{
	std::string message = "Usuario " + getUserName(client_fd) + " se ha unido al canal " + channel;
	std::cout << message << std::endl;
	#ifdef BONUS_MODE
	int	bot_id = getClientFdByNickname(socket_manager.getNicknames(), "HAL9000 🤖");
	socket_manager.sendMessageToClient(bot_id, message);
	#endif

    user_channels[client_fd].insert(channel);
}

void UserManager::removeUserChannel(int client_fd, const std::string& channel)
{
    if (user_channels.find(client_fd) != user_channels.end())
    {
        user_channels[client_fd].erase(channel);
        if (user_channels[client_fd].empty())
        {
            user_channels.erase(client_fd);
        }
    }
}

std::set<std::string> UserManager::getUserChannels(int client_fd) const
{
    if (user_channels.find(client_fd) != user_channels.end())
    {
        return user_channels.at(client_fd);
    }
    return std::set<std::string>();
}

std::set<std::string> UserManager::findChannelsByUsername(const std::string& username) const
{
    std::set<std::string> result;
    for (std::map<int, std::string>::const_iterator it = usernames.begin(); it != usernames.end(); ++it)
    {
        if (it->second == username)
        {
            if (user_channels.find(it->first) != user_channels.end())
            {
                result = user_channels.at(it->first);
            }
            break;
        }
    }
    return result;
}

std::set<std::string> UserManager::findChannelsByNickname(const std::string& nickname, const std::map<int, std::string>& nicknames) const
{
    std::set<std::string> result;
    for (std::map<int, std::string>::const_iterator it = nicknames.begin(); it != nicknames.end(); ++it)
    {
        if (it->second == nickname)
        {
            if (user_channels.find(it->first) != user_channels.end())
            {
                result = user_channels.at(it->first);
            }
            break;
        }
    }
    return result;
}

void UserManager::setActiveChannel(int client_fd, const std::string& channel)
{
    active_channel[client_fd] = channel;
}

std::string UserManager::getActiveChannel(int client_fd) const
{
    std::map<int, std::string>::const_iterator it = active_channel.find(client_fd);
    
    if (it != active_channel.end())
    {
        return it->second;
    }
    return "";
}

void UserManager::removeActiveChannel(int client_fd)
{
    active_channel.erase(client_fd);    
}
