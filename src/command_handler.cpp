/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_handler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:00:56 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/08 17:46:36 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/command_handler.h"
#include "../includes/socket_manager.h"
#include "../includes/channel_manager.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

std::map<std::string, Channel> channels;

/*	Constructor de CommandHandler.
	Se inicializan los atributos server_password, nicknames, authenticated_clients, user_manager y socket_manager.
	Se rellena el mapa commandMap con los comandos y su correspondiente tipo.
	Se muestra el contenido de commandMap.
*/
CommandHandler::CommandHandler(const std::string& server_password, std::map<int, std::string>& nicknames,
std::set<int>& authenticated_clients, UserManager& user_manager, SocketManager& socket_manager) :
	server_password(server_password),
	authenticated_clients(authenticated_clients),
    nicknames(nicknames),
    user_manager(user_manager),
    socket_manager(socket_manager)
{
    commandMap["/PASS"] = CMD_PASS;
    commandMap["/NICK"] = CMD_NICK;
    commandMap["/USER"] = CMD_USER;
    commandMap["/JOIN"] = CMD_JOIN;
    commandMap["/PART"] = CMD_PART;
    commandMap["/NAMES"] = CMD_NAMES;
    commandMap["/LIST"] = CMD_LIST;
    commandMap["/KICK"] = CMD_KICK;
	commandMap["/PRIVMSG"] = CMD_PRIVMSG;
	commandMap["/MODE"] = CMD_MODE;
	commandMap["/INVITE"] = CMD_INVITE;
	commandMap["/TOPIC"] = CMD_TOPIC;
	commandMap["/WHEREIS"] = CMD_WHEREIS;
	commandMap["/WHEREAMI"] = CMD_WHEREAMI;
	commandMap["/ACTIVE"] = CMD_ACTIVE;
}

/*	Destructor de CommandHandler.
*/
CommandHandler::~CommandHandler()
{
}

/*	Se encarga de procesar un comando.
	1.-	Se comprueba si el comando está vacío o no comienza con '/' (doble seguridad, ya viene comprobado).
	2.-	Se obtiene el nombre del comando y sus argumentos.
	3.-	Se comprueba si el comando está en el mapa commandMap.
	4.-	Se llama a la función correspondiente según el tipo de comando.
*/
void CommandHandler::handleCommand(int client_fd, const std::string& command)
{
    if (command.empty() || command[0] != '/')
    {
        socket_manager.sendMessageToClient(client_fd, "Error: Los comandos deben comenzar con '/'.\n");
        return;
    }

    size_t spacePos = command.find(' ');
    std::string cmdName = (spacePos != std::string::npos) ? command.substr(0, spacePos) : command;
    std::string cmdArgs = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";

    std::cout << "cmdName: " << cmdName << std::endl;

    CommandType cmdType = CMD_UNKNOWN;
    if (commandMap.find(cmdName) != commandMap.end())
    {
        cmdType = commandMap[cmdName];
    }

    switch (cmdType)
    {
        case CMD_PASS:
        {
            handlePassCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_NICK:
        {
            handleNickCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_USER:
        {
            handleUserCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_JOIN:
        {
            joinChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_PART:
        {
            partChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_NAMES:
        {
            listUsersInChannel(client_fd, cmdArgs.substr(0, cmdArgs.find('\n')));
            break;
        }
        case CMD_LIST:
        {
            listChannels(client_fd);
            break;
        }
        case CMD_KICK:
        {
            handleKickCommand(client_fd, cmdArgs);
            break;
        }
        case CMD_PRIVMSG:
		{
			handlePrivMsgCommand(client_fd, cmdArgs);
			break;
		}
		case CMD_MODE:
		{
			handleModeCommand(client_fd, cmdArgs);
			break;
		}
		case CMD_INVITE:
		{
			handleInviteCommand(client_fd, cmdArgs);
			break;
		}
		case CMD_TOPIC:
		{
			handleTopicCommand(client_fd, cmdArgs);
			break;
		}
		case CMD_WHEREIS:
    	{
			handleWhereIsCommand(client_fd, cmdArgs);
    		break;
		}
		case CMD_WHEREAMI:
		{
			handleWhereAmICommand(client_fd, cmdArgs);
			break;
		}
		case CMD_ACTIVE:
		{
			handleActiveCommand(client_fd, cmdArgs);
			break;
		}
        default:
            break;
    }
}

#include "authentication_commands.cpp"
#include "channel_commands.cpp"
