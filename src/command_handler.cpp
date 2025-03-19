/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_handler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:00:56 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/19 18:46:13 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/command_handler.h"
#include "../includes/socket_manager.h"
#include "../includes/channel_manager.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

std::map<std::string, Channel> channels;

CommandHandler::CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames, std::set<int>& authenticated_clients, UserManager& user_manager, SocketManager& socket_manager)
    : server_password(server_password), nicknames(nicknames), authenticated_clients(authenticated_clients), user_manager(user_manager), socket_manager(socket_manager)
    {
    commandMap["PASS"] = CMD_PASS;
    commandMap["NICK"] = CMD_NICK;
    commandMap["USER"] = CMD_USER;
    commandMap["/JOIN"] = CMD_JOIN; // Nuevo
    commandMap["/PART"] = CMD_PART; // Nuevo
    commandMap["/NAMES"] = CMD_NAMES; // Nuevo
    commandMap["/LIST"] = CMD_LIST; // Nuevo
    commandMap["/KICK"] = CMD_KICK; // Nuevo
    std::cout << "Contenido de commandMap:" << std::endl;
    for (std::map<std::string, CommandType>::const_iterator it = commandMap.begin(); it != commandMap.end(); ++it) {
        std::cout << "Comando: " << it->first << ", Tipo: " << it->second << std::endl;
    }
}

void CommandHandler::handleCommand(int client_fd, const std::string& command) {
    size_t spacePos = command.find(' ');
    std::string cmdName = command.substr(0, spacePos);
    std::string cmdArgs = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";

    std::cout << "cmdName: " << cmdName << std::endl;

    CommandType cmdType = CMD_UNKNOWN;
    if (commandMap.find(cmdName) != commandMap.end()) {
        cmdType = commandMap[cmdName];
    }

    switch (cmdType) {
        case CMD_PASS: {
            std::string client_password = cmdArgs.substr(0, cmdArgs.find('\n'));
            if (client_password == server_password) {
                send(client_fd, "Contraseña correcta. Bienvenido al servidor IRC.\n", 50, 0);
                nicknames[client_fd] = "Invitado";
                authenticated_clients.insert(client_fd);
            } else {
                send(client_fd, "Contraseña incorrecta. Inténtalo de nuevo.\n", 38, 0);
                close(client_fd);
            }
            break;
        }
        case CMD_NICK: {
            if (authenticated_clients.find(client_fd) != authenticated_clients.end()) {
                std::string new_nickname = cmdArgs.substr(0, cmdArgs.find('\n'));
                nicknames[client_fd] = new_nickname;
                socket_manager.sendMessageToClient(client_fd, ("Nickname cambiado a " + new_nickname + ".\n"));
            } else {
                socket_manager.sendMessageToClient(client_fd, "Debes autenticarte antes de cambiar tu nickname.\n");
            }
            break;
        }
        case CMD_USER: {
            if (authenticated_clients.find(client_fd) != authenticated_clients.end()) {
                std::string username = cmdArgs.substr(0, cmdArgs.find(' '));
                user_manager.setUserName(client_fd, username);
                socket_manager.sendMessageToClient(client_fd, ("Username establecido a " + username + ".\n"));
            } else {
                socket_manager.sendMessageToClient(client_fd, "Debes autenticarte antes de establecer tu username.\n");
            }
            break;
        }
        case CMD_JOIN: {
            joinChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_PART: {
            partChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_NAMES: {
            listUsersInChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_LIST: {
            std::cout << "llamando a listChannels" << std::endl;
            listChannels(client_fd);
            break;
        }
        case CMD_KICK: {
            size_t spacePosKick = cmdArgs.find(' ');
            std::string userName = cmdArgs.substr(0, spacePosKick);
            std::string channelName = cmdArgs.substr(spacePosKick + 1, cmdArgs.find('\n'));
            kickUserFromChannel(client_fd, userName, channelName);
            break;
        }
        default:
            break;
    }
}

void CommandHandler::joinChannel(int client_fd, const std::string& channelName) {
    if (channels.find(channelName) == channels.end()) {
        Channel newChannel;
        newChannel.name = channelName;
        newChannel.users.insert(client_fd);
        newChannel.creator = client_fd;
        channels[channelName] = newChannel;
    } else {
        channels[channelName].users.insert(client_fd);
    }
    user_manager.setUserChannel(client_fd, channelName); // Añadir esta línea
    socket_manager.sendMessageToClient(client_fd, "Te has unido al canal " + channelName + ".\n");
}

void CommandHandler::partChannel(int client_fd, const std::string& channelName) {
    if (channels.find(channelName) != channels.end()) {
        channels[channelName].users.erase(client_fd);
        socket_manager.sendMessageToClient(client_fd, "Has abandonado el canal " + channelName + ".\n");
        if (channels[channelName].users.empty()) {
            channels.erase(channelName);
        }
    } else {
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

void CommandHandler::listUsersInChannel(int client_fd, const std::string& channelName) {
    if (channels.find(channelName) != channels.end()) {
        std::string userList = "Usuarios en " + channelName + ": ";
        std::set<int>::iterator it;
        for (it = channels[channelName].users.begin(); it != channels[channelName].users.end(); ++it) {
            userList += nicknames[*it] + " ";
        }
        userList += "\n";
        socket_manager.sendMessageToClient(client_fd, userList);
    } else {
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

void CommandHandler::listChannels(int client_fd) {
    std::cout << "Inicio de listChannels" << std::endl;
    std::cout << "Contenido del mapa channels:" << std::endl;
    for (std::map<std::string, Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
        std::cout << "Canal: " << it->first << std::endl;
    }
    std::cout << "Fin de primer bucle for" << std::endl;

    std::string channelList = "Canales disponibles: #";
    std::map<std::string, Channel>::const_iterator it;
    for (it = channels.begin(); it != channels.end(); ++it) {
        channelList += it->first + ", #";
    }
    channelList += "\r\n";
    std::cout << "Fin de segundo bucle for" << std::endl;
    std::cout << "Lista de canales: " << channelList;
    std::cout << "Tamaño de channels: " << channels.size() << std::endl;
    std::cout << "Tamaño de channelList: " << channelList.length() << std::endl;
    std::cout << "Llamando a sendMessageToClient" << std::endl;
    socket_manager.sendMessageToClient(client_fd, channelList);
    std::cout << "sendMessageToClient llamado" << std::endl;
    std::cout << "Fin de listChannels" << std::endl;
}

void CommandHandler::kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName) {
    if (channels.find(channelName) != channels.end()) {
        if (channels[channelName].creator == client_fd) {
            int userToKickFd = -1;
            std::map<int, std::string>::const_iterator it;
            for (it = nicknames.begin(); it != nicknames.end(); ++it) {
                if (it->second == userName) {
                    userToKickFd = it->first;
                    break;
                }
            }
            if (userToKickFd != -1) {
                channels[channelName].users.erase(userToKickFd);
                socket_manager.sendMessageToClient(userToKickFd, "Has sido expulsado del canal " + channelName + ".\n");
                socket_manager.sendMessageToClient(client_fd, "Usuario " + userName + " expulsado del canal " + channelName + ".\n");
            } else {
                socket_manager.sendMessageToClient(client_fd, "Usuario " + userName + " no encontrado.\n");
            }
        } else {
            socket_manager.sendMessageToClient(client_fd, "No tienes permiso para expulsar usuarios de este canal.\n");
        }
    } else {
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

const std::map<std::string, Channel>& CommandHandler::getChannels() const {
    return channels;
}