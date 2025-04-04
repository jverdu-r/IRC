/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user_manager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:15 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/02 22:57:20 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/user_manager.h"
#include <iostream>

/*	Constructor de UserManager.
	Se inicializa el atributo usernames.
*/
UserManager::UserManager(std::map<int, std::string>& usernames) :
	usernames(usernames)
{
}

/*	Destructor de UserManager.
*/
UserManager::~UserManager()
{
}

/*	Asigna un nombre de usuario a un cliente.
*/
void UserManager::setUserName(int client_fd, const std::string& username)
{
    usernames[client_fd] = username;
}

/*	Devuelve el nombre de usuario de un cliente.
*/
std::string UserManager::getUserName(int client_fd)
{
    if (usernames.find(client_fd) != usernames.end())
    {
        return usernames[client_fd];
    }
    return "";
}

/*	Comprueba si un nombre de usuario ya está en uso.
*/
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

/*	Agrega un canal a un cliente.
*/
void UserManager::addUserChannel(int client_fd, const std::string& channel)
{
    user_channels[client_fd].insert(channel);
    std::cout << "Usuario " << client_fd << " se ha unido al canal: " << channel << std::endl;
}

/*	Elimina un canal de un cliente.
*/
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

/*	Devuelve los canales de un cliente.
*/
std::set<std::string> UserManager::getUserChannels(int client_fd) const
{
    if (user_channels.find(client_fd) != user_channels.end())
    {
        return user_channels.at(client_fd);
    }
    return std::set<std::string>();
}

/*	Devuelve los canales de un cliente a partir de su nombre de usuario.
*/
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

/*	Devuelve los canales de un cliente a partir de su apodo.
*/
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
