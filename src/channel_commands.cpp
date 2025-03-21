#include "command_handler.h"
#include <iostream>
#include <string>
#include <set>

void CommandHandler::kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
    {
        if (channels[channelName].creator == client_fd)
        {
            int userToKickFd = -1;
            std::map<int, std::string>::const_iterator it;
            for (it = nicknames.begin(); it != nicknames.end(); ++it)
            {
                if (it->second == userName)
                {
                    userToKickFd = it->first;
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

void CommandHandler::listChannels(int client_fd)
{
    //std::cout << "Inicio de listChannels" << std::endl;
    //std::cout << "Contenido del mapa channels:" << std::endl;
    /*for (std::map<std::string, Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
        std::cout << "Canal: " << it->first << std::endl;
    }*/
    //std::cout << "Fin de primer bucle for" << std::endl;

    std::string channelList = "Canales disponibles: #";
    std::map<std::string, Channel>::const_iterator it;
    for (it = channels.begin(); it != channels.end(); ++it)
    {
        channelList += it->first + ", #";
    }
    if (!channels.empty())
    {
        channelList.erase(channelList.length() - 2, 2);
    }

    channelList += "\r\n";
    //std::cout << "Fin de segundo bucle for" << std::endl;
    //std::cout << "Lista de canales: " << channelList;
    //std::cout << "Tamaño de channels: " << channels.size() << std::endl;
    //std::cout << "Tamaño de channelList: " << channelList.length() << std::endl;
    //std::cout << "Llamando a sendMessageToClient" << std::endl;
    socket_manager.sendMessageToClient(client_fd, channelList);
    //std::cout << "sendMessageToClient llamado" << std::endl;
    //std::cout << "Fin de listChannels" << std::endl;
}

void CommandHandler::joinChannel(int client_fd, const std::string& channelName)
{
    if (channels.find(channelName) == channels.end())
    {
        Channel newChannel;
        newChannel.name = channelName;
        newChannel.users.insert(client_fd);
        newChannel.creator = client_fd;
        channels[channelName] = newChannel;
    }
    else
    {
        channels[channelName].users.insert(client_fd);
    }
    user_manager.setUserChannel(client_fd, channelName);
    socket_manager.sendMessageToClient(client_fd, "Te has unido al canal " + channelName + ".\n");
}

void CommandHandler::partChannel(int client_fd, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
    {
        channels[channelName].users.erase(client_fd);
        socket_manager.sendMessageToClient(client_fd, "Has abandonado el canal " + channelName + ".\n");
        if (channels[channelName].users.empty())
        {
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
        std::set<int>::iterator it;
        for (it = channels[channelName].users.begin(); it != channels[channelName].users.end(); ++it)
        {
            userList += nicknames[*it] + " ";
        }
        userList += "\n";
        socket_manager.sendMessageToClient(client_fd, userList);
    }
    else
    {
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

const std::map<std::string, Channel>& CommandHandler::getChannels() const
{
    return channels;
}

void CommandHandler::handleKickCommand(int client_fd, const std::string& cmdArgs)
{
    size_t spacePosKick = cmdArgs.find(' ');
    std::string userName = cmdArgs.substr(0, spacePosKick);
    std::string channelName = cmdArgs.substr(spacePosKick + 1, cmdArgs.find('\n'));
    kickUserFromChannel(client_fd, userName, channelName);
}
