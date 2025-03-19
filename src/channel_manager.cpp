/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_manager.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:00:47 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 16:00:50 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

void CommandHandler::joinChannel(int client_fd, const std::string& channelName)
{
    if (channels.find(channelName) == channels.end())
	{
        // El canal no existe, crearlo
        Channel newChannel;
        newChannel.name = channelName;
        newChannel.users.insert(client_fd);
        newChannel.creator = client_fd;
        channels[channelName] = newChannel;
    }
	else
	{
        // El canal existe, unirse
        channels[channelName].users.insert(client_fd);
    }
    socket_manager.sendMessageToClient(client_fd, "Te has unido al canal " + channelName + ".\n");
}

void CommandHandler::partChannel(int client_fd, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
	{
        channels[channelName].users.erase(client_fd);
        socket_manager.sendMessageToClient(client_fd, "Has abandonado el canal " + channelName + ".\n");
        if (channels[channelName].users.empty()) {
            // Eliminar el canal si no hay usuarios
            channels.erase(channelName);
        }
    }
	else
	{
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

void CommandHandler::listUsersInChannel(int client_fd, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
	{
        std::string userList = "Usuarios en " + channelName + ": ";
        for (int userFd : channels[channelName].users)
		{
            userList += nicknames[userFd] + " ";
        }
        userList += "\n";
        socket_manager.sendMessageToClient(client_fd, userList);
    }
	else
	{
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

void CommandHandler::listChannels(int client_fd)
{
    std::string channelList = "Canales disponibles: ";
    for (auto const& [channelName, channel] : channels)
	{
        channelList += channelName + " ";
    }
    channelList += "\n";
    socket_manager.sendMessageToClient(client_fd, channelList);
}

void CommandHandler::kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
	{
        if (channels[channelName].creator == client_fd)
		{
            int userToKickFd = -1;
            for (auto const& [fd, nick] : nicknames)
			{
                if (nick == userName)
				{
                    userToKickFd = fd;
                    break;
                }
            }
            if (userToKickFd != -1)
			{
                channels[channelName].users.erase(userToKickFd);
                socket_manager.sendMessageToClient(userToKickFd, "Has sido expulsado del canal " + channelName + ".\n");
                socket_manager.sendMessageToClient(client_fd, "Usuario " + userName + " expulsado del canal " + channelName + ".\n");
            }
			else
			{
                socket_manager.sendMessageToClient(client_fd, "Usuario " + userName + " no encontrado.\n");
            }
        }
		else
		{
            socket_manager.sendMessageToClient(client_fd, "No tienes permiso para expulsar usuarios de este canal.\n");
        }
    }
	else
	{
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}