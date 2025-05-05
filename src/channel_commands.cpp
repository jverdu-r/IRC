#include "command_handler.h"
#include "socket_manager.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <set>
#include <sstream>

void CommandHandler::listChannels(int client_fd)
{
    std::string channelList = "Canales disponibles:\n";
    std::set<std::string> userChannels = user_manager.getUserChannels(client_fd);
    std::string activeChannel = user_manager.getActiveChannel(client_fd);

    if (channels.empty())
    {
        channelList += "No hay ningún canal.\n";
    }
    else
    {
        for (std::map<std::string, Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
        {
            const std::string& channelName = it->first;
            const Channel& channel = it->second;

            channelList += "#" + channelName;
            if (userChannels.find(channelName) != userChannels.end())
            {
                channelList += " (Estás dentro)";
                if (channelName == activeChannel)
                {
                    channelList += " - Activo!!";
                }
            }
            std::stringstream ss;
			ss << " — " << channel.users.size() << " usuarios";
			channelList += ss.str();
            if (!channel.topic.empty())
            {
                channelList += " — " + channel.topic;
            }
            channelList += "\n";
        }
    }

    socket_manager.sendMessageToClient(client_fd, channelList);
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
		user_manager.setActiveChannel(client_fd, channelName);
		#ifdef BONUS_MODE
		socket_manager.sendMessageToClient(getClientFdByNickname(nicknames, "HAL9000 🤖"), "Se ha creado el canal " + channelName + "\n");
		#endif
    }
    else
    {
        channels[channelName].users.insert(client_fd);
		user_manager.setActiveChannel(client_fd, channelName);
    }
    user_manager.addUserChannel(client_fd, channelName);
    socket_manager.sendMessageToClient(client_fd, "Te has unido al canal " + channelName + "\n");
}

void CommandHandler::partChannel(int client_fd, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
    {
        channels[channelName].users.erase(client_fd);
		user_manager.removeUserChannel(client_fd, channelName);
        socket_manager.sendMessageToClient(client_fd, "Has abandonado el canal " + channelName + ".\n");
		#ifdef BONUS_MODE
		socket_manager.sendMessageToClient(getClientFdByNickname(nicknames, "HAL9000 🤖"), user_manager.getUserName(client_fd) + " ha abandonado el canal " + channelName + "\n");
		#endif
        if (channels[channelName].users.empty())
        {
            channels.erase(channelName);
			if (user_manager.getActiveChannel(client_fd) == channelName)
			{

				std::set<std::string> remainingChannels = user_manager.getUserChannels(client_fd);
				if (!remainingChannels.empty())
				{
					user_manager.setActiveChannel(client_fd, *remainingChannels.begin());
                    socket_manager.sendMessageToClient(client_fd, "El canal activo pasa a ser: " + *remainingChannels.begin() + "\n");
				}
                else
                {
				    user_manager.removeActiveChannel(client_fd);
                }
			}
        }
        else
        {
			if (user_manager.getActiveChannel(client_fd) == channelName)
			{

				std::set<std::string> remainingChannels = user_manager.getUserChannels(client_fd);
				if (!remainingChannels.empty())
				{
					user_manager.setActiveChannel(client_fd, *remainingChannels.begin());
                    socket_manager.sendMessageToClient(client_fd, "El canal activo pasa a ser: " + *remainingChannels.begin() + "\n");
				}
                else
                {
				    user_manager.removeActiveChannel(client_fd);
                }
			}
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
        std::string userList = "Usuarios en " + channelName + ": \n";
        std::set<int>::iterator it;
        for (it = channels[channelName].users.begin(); it != channels[channelName].users.end(); ++it)
        {    
			if (channels[channelName].operators.count(*it))
			{
				userList += "@" + nicknames[*it] + "\n";
			}
			else
			{
				userList += nicknames[*it] + "\n";
			}
        }
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

void CommandHandler::kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
    {
        if (channels[channelName].creator == client_fd || channels[channelName].operators.count(client_fd))
        {
            int userToKickFd = -1;
            std::map<int, std::string>::const_iterator it;
            for (it = usernames.begin(); it != usernames.end(); ++it)
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
        socket_manager.sendMessageToClient(client_fd, "Uso: /KICK <usuario> <canal>\n");
    }
}

void CommandHandler::handlePrivMsgCommand(int client_fd, const std::string& cmdArgs)
{
    size_t spacePosPriv = cmdArgs.find(' ');
    if (spacePosPriv == std::string::npos)
    {
        socket_manager.sendMessageToClient(client_fd, "Uso: /PRIVMSG <usuario|#canal> <mensaje>\n");
        return;
    }

    std::string target = cmdArgs.substr(0, spacePosPriv);
    std::string message = cmdArgs.substr(spacePosPriv + 1);

    if (target.empty())
    {
        socket_manager.sendMessageToClient(client_fd, "Falta Usuario o Canal. Uso: /PRIVMSG <usuario|#canal> <mensaje>\n");
        return;
    }
    if (message.empty())
    {
        socket_manager.sendMessageToClient(client_fd, "Falta Mensaje. Uso: /PRIVMSG <usuario|#canal> <mensaje>\n");
        return;
    }

    std::string sender_nick = nicknames[client_fd];
    std::string sender_user = user_manager.getUserName(client_fd);
    std::string formatted_message = "PRIVMSG " + sender_user + "!" + sender_nick + " -> " + message + "\n";

    if (target[0] == '#')
    {
		target = target.substr(1);
        const std::map<std::string, Channel>& channels = getChannels();
        if (channels.find(target) != channels.end())
        {
            const std::set<int>& users = channels.at(target).users;
            for (std::set<int>::const_iterator it = users.begin(); it != users.end(); ++it)
            {
                if (*it != client_fd)
                {
					socket_manager.sendMessageToClient(*it, formatted_message);
				}
            }
        }
        std::cout << "opcion con #\n";
    }
    else if (target[0] != '#')
    {
        int target_fd = -1;
        for (std::map<int, std::string>::const_iterator it = usernames.begin(); it != usernames.end(); ++it)
        {
            if (it->second == target)
			{
				target_fd = it->first;
				break;
			}
        }

        if (target_fd != -1)
        {
            socket_manager.sendMessageToClient(target_fd, formatted_message);
        }
        else
        {
            socket_manager.sendMessageToClient(client_fd, "Usuario o canal " + target + " no encontrado.\n");
        }
        std::cout << "opcion sin #\n";
    }
}

void CommandHandler::handleModeCommand(int client_fd, const std::string& cmdArgs)
{
    std::istringstream iss(cmdArgs);
    std::string channelName, mode, targetUser;
    iss >> channelName >> mode >> targetUser;

    if (channelName.empty() || mode.empty() || targetUser.empty())
	{
        socket_manager.sendMessageToClient(client_fd, "Uso: /MODE <canal> <+o|-o> <usuario>\n");
        return;
    }

    if (channels.find(channelName) == channels.end())
	{
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
        return;
    }

    Channel& channel = channels[channelName];

    int target_fd = -1;
    for (std::map<int, std::string>::iterator it = usernames.begin(); it != usernames.end(); ++it)
	{
        if (it->second == targetUser)
		{
            target_fd = it->first;
            break;
        }
    }

    if (target_fd == -1)
	{
        socket_manager.sendMessageToClient(client_fd, "Usuario " + targetUser + " no encontrado.\n");
        return;
    }

    if (mode == "+o")
    {
        if (channel.creator != client_fd && channel.operators.count(client_fd) == 0)
        {
            socket_manager.sendMessageToClient(client_fd, "No tienes permiso para añadir operadores.\n");
            return;
        }

        channel.operators.insert(target_fd);
        socket_manager.sendMessageToClient(client_fd, "Usuario " + targetUser + " ahora es operador del canal " + channelName + ".\n");
        socket_manager.sendMessageToClient(target_fd, "Has sido asignado como operador del canal " + channelName + ".\n");
    }
    else if (mode == "-o")
    {
        if (channel.creator != client_fd)
        {
            socket_manager.sendMessageToClient(client_fd, "Solo el creador del canal puede quitar operadores.\n");
            return;
        }

        channel.operators.erase(target_fd);
        socket_manager.sendMessageToClient(client_fd, "Usuario " + targetUser + " ha sido removido como operador del canal " + channelName + ".\n");
        socket_manager.sendMessageToClient(target_fd, "Has sido removido como operador del canal " + channelName + ".\n");
    }
    else
    {
        socket_manager.sendMessageToClient(client_fd, "Modo desconocido. Uso: /MODE <canal> <+o|-o> <usuario>\n");
    }
}

void CommandHandler::handleInviteCommand(int client_fd, const std::string& cmdArgs)
{
    size_t spacePos = cmdArgs.find(' ');
    if (spacePos == std::string::npos)
	{
        socket_manager.sendMessageToClient(client_fd, "Uso: /INVITE <usuario> <canal>\n");
        return;
    }

    std::string targetUser = cmdArgs.substr(0, spacePos);
    std::string channelName = cmdArgs.substr(spacePos + 1);

    if (channels.find(channelName) == channels.end())
	{
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
        return;
    }
    Channel& channel = channels[channelName];
    if (channel.creator != client_fd && channel.operators.count(client_fd) == 0)
	{
        socket_manager.sendMessageToClient(client_fd, "No tienes permiso para invitar usuarios a este canal.\n");
        return;
    }

    int target_fd = -1;
    for (std::map<int, std::string>::iterator it = usernames.begin(); it != usernames.end(); ++it)
	{
        if (it->second == targetUser)
		{
            target_fd = it->first;
            break;
        }
    }
    if (target_fd == -1)
	{
        socket_manager.sendMessageToClient(client_fd, "Usuario " + targetUser + " no encontrado.\n");
        return;
    }

    channel.users.insert(target_fd);
    user_manager.addUserChannel(target_fd, channelName);
    socket_manager.sendMessageToClient(client_fd, "Has invitado a " + targetUser + " al canal " + channelName + ".\n");
    socket_manager.sendMessageToClient(target_fd, "Has sido invitado al canal " + channelName + ".\n");
}

void CommandHandler::handleTopicCommand(int client_fd, const std::string& cmdArgs)
{
    size_t spacePos = cmdArgs.find(' ');
    std::string channelName = cmdArgs.substr(0, spacePos);
    std::string newTopic = (spacePos != std::string::npos) ? cmdArgs.substr(spacePos + 1) : "";

    if (channels.find(channelName) == channels.end())
	{
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
        return;
    }

    Channel& channel = channels[channelName];
    if (newTopic.empty())
	{
        if (channel.topic.empty())
            socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no tiene tema establecido.\n");
        else
            socket_manager.sendMessageToClient(client_fd, "Tema actual de " + channelName + ": " + channel.topic + "\n");
        return;
    }

    if (channel.creator != client_fd && channel.operators.count(client_fd) == 0)
	{
        socket_manager.sendMessageToClient(client_fd, "No tienes permiso para cambiar el tema de este canal.\n");
        return;
    }

    channel.topic = newTopic;
    socket_manager.sendMessageToClient(client_fd, "Tema del canal " + channelName + " cambiado a: " + newTopic + "\n");
    for (std::set<int>::iterator it = channel.users.begin(); it != channel.users.end(); ++it)
	{
        if (*it != client_fd)
		{
            socket_manager.sendMessageToClient(*it, "El tema del canal " + channelName + " ha sido cambiado a: " + newTopic + "\n");
        }
    }
}

void CommandHandler::handleWhereIsCommand(int client_fd, const std::string& cmdArgs)
{
    if (cmdArgs.empty())
    {
        socket_manager.sendMessageToClient(client_fd, "Uso: /WHEREIS <username|nickname>\n");
        return;
    }

    std::set<std::string> channelsByUsername = user_manager.findChannelsByUsername(cmdArgs);
    std::set<std::string> channelsByNickname = user_manager.findChannelsByNickname(cmdArgs, nicknames);

    std::set<std::string> resultChannels;
    resultChannels.insert(channelsByUsername.begin(), channelsByUsername.end());
    resultChannels.insert(channelsByNickname.begin(), channelsByNickname.end());

    if (resultChannels.empty())
    {
        socket_manager.sendMessageToClient(client_fd, "No se encontraron canales para " + cmdArgs + ".\n");
        return;
    }

    std::string response = "Canales de " + cmdArgs + ": \n";
    for (std::set<std::string>::iterator it = resultChannels.begin(); it != resultChannels.end(); ++it)
    {
        response += *it + "\n";
    }
    response += "\n";

    socket_manager.sendMessageToClient(client_fd, response);
}

void CommandHandler::handleWhereAmICommand(int client_fd, const std::string& cmdArgs)
{
    (void)cmdArgs;

    std::set<std::string> userChannels = user_manager.getUserChannels(client_fd);
    if (userChannels.empty())
    {
        socket_manager.sendMessageToClient(client_fd, "No estás en ningún canal.\n");
        return;
    }

    std::string response = "Estás en los siguientes canales:\n";
    std::string activeChannel = user_manager.getActiveChannel(client_fd);
    for (std::set<std::string>::iterator it = userChannels.begin(); it != userChannels.end(); ++it)
    {
        const std::string& channelName = *it;
        const Channel& channel = channels[channelName];
        response += "# " + channelName;
        if (channelName == activeChannel)
        {
            response += " (Activo)";
        }
        if (channel.operators.find(client_fd) != channel.operators.end())
        {
            response += " (Operador)";
        }
        response += "\n";
    }
    socket_manager.sendMessageToClient(client_fd, response);
}

void CommandHandler::handleActiveCommand(int client_fd, const std::string& cmdArgs)
{
    if (cmdArgs.empty())
    {
        socket_manager.sendMessageToClient(client_fd, "Uso: /ACTIVE <canal>\n");
        return;
    }

    std::string channelName = cmdArgs;
    std::set<std::string> userChannels = user_manager.getUserChannels(client_fd);
    if (userChannels.find(channelName) == userChannels.end())
    {
        socket_manager.sendMessageToClient(client_fd, "No estás en el canal " + channelName + ".\n");
        return;
    }
    user_manager.setActiveChannel(client_fd, channelName);
    socket_manager.sendMessageToClient(client_fd, "Canal activo cambiado a " + channelName + ".\n");
}

void CommandHandler::handleHelpCommand(int client_fd)
{
    send(client_fd, "/PASS\t\t->\tintroduce el password\n", strlen("/PASS\t\t->\tintroduce el password\n"), 0);
    send(client_fd, "/NICK\t\t->\tpara cambiar tu apodo\n", strlen("/NICK\t\t->\tpara cambiar tu apodo\n"), 0);
    send(client_fd, "/USER\t\t->\tpara cambiar tu usuario\n", strlen("/USER\t\t->\tpara cambiar tu usuario\n"), 0);
    send(client_fd, "/JOIN\t\t->\tpara unirte a un canal\n", strlen("/JOIN\t\t->\tpara unirte a un canal\n"), 0);
    send(client_fd, "/PART\t\t->\tpara salir de un canal\n", strlen("/PART\t\t->\tpara salir de un canal\n"), 0);
    send(client_fd, "/KICK\t\t->\tpara echar a un usuario de un canal\n", strlen("/KICK\t\t->\tpara echar a un usuario de un canal\n"), 0);
    send(client_fd, "/NAMES\t\t->\tlista los usuarios de un canal\n", strlen("/NAMES\t\t->\tlista los usuarios de un canal\n"), 0);
    send(client_fd, "/LIST\t\t->\tlista todos los canales\n", strlen("/LIST\t\t->\tlista todos los canales\n"), 0);
    send(client_fd, "/PRIVMSG\t->\tenvia un mensaje privado a un canal o usuario\n", strlen("/PRIVMSG\t->\tenvia un mensaje privado a un canal o usuario\n"), 0);
    send(client_fd, "/MODE\t\t->\tda (+o) o retira (-o) permisos de operador\n", strlen("/MODE\t\t->\tda (+o) o retira (-o) permisos de operador\n"), 0);
    send(client_fd, "/INVITE\t\t->\tinvita a un usuario al canal\n", strlen("/INVITE\t\t->\tinvita a un usuario al canal\n"), 0);
    send(client_fd, "/TOPIC\t\t->\tcambia la descripción de un canal \n", strlen("/TOPIC\t\t->\tcambia la descripción de un canal \n"), 0);
    send(client_fd, "/WHEREIS\t->\tlista los canales en los que está un usuario\n", strlen("/WHEREIS\t->\tlista los canales en los que está un usuario\n"), 0);
    send(client_fd, "/WHEREAMI\t->\tlista los canales en los que estás\n", strlen("/WHEREAMI\t->\tlista los canales en los que estás\n"), 0);
    send(client_fd, "/ACTIVE\t\t->\tcambia tu canal activo (en el que escribes)\n", strlen("/ACTIVE\t\t->\tcambia tu canal activo (en el que escribes)\n"), 0);
    send(client_fd, "/HELP\t\t->\tmuestra la tabla de comandos\n", strlen("/HELP\t\t->\tmuestra la tabla de comandos\n"), 0);
    send(client_fd, " \n", strlen(" \n"), 0);
}