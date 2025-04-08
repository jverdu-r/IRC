#include "command_handler.h"
#include "socket_manager.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <set>
#include <sstream>

/*	Comando /LIST.
	Se listan los canales en losque está el cliente y se envía un mensaje al cliente con la lista.
*/
void CommandHandler::listChannels(int client_fd)
{
    std::string channelList = "Canales disponibles:\n";
    std::set<std::string> userChannels = user_manager.getUserChannels(client_fd);
    std::string activeChannel = user_manager.getActiveChannel(client_fd);

    if (channels.empty())
    {
        channelList += "No estás en ningún canal.\n";
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

/*	Commando /JOIN.
	1.-	Se comprueba si el canal no existe.
		 -	Si no existe, se crea un nuevo canal y se añade el usuario.
		 -	Si el canal existe, se añade el usuario al canal.
	2.-	Se actualiza el canal del usuario, y se envía un mensaje al usuario.
*/
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

/*	Comando /PART.
	1.-	Se comprueba si el canal existe.
		 -	Si existe, se elimina al usuario del canal y se envía un mensaje al usuario.
		 -	Si el canal queda vacío, se elimina del mapa de canales.
*/
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
				user_manager.removeActiveChannel(client_fd);

				std::set<std::string> remainingChannels = user_manager.getUserChannels(client_fd);
				if (!remainingChannels.empty())
				{
					user_manager.setActiveChannel(client_fd, *remainingChannels.begin());
				}
			}
        }
    }
    else
    {
        socket_manager.sendMessageToClient(client_fd, "El canal " + channelName + " no existe.\n");
    }
}

/*	Comando /NAMES.
	1.-	Se comprueba si el canal existe.
		 -	Si existe, se recorre el conjunto de usuarios del canal y se envía un mensaje al cliente
			con los nombres de los usuarios.
		 -	Si no existe, se informa al cliente.
*/
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

/*	Comando /LIST.
	1.-	Se devuelve el mapa de canales.
*/
const std::map<std::string, Channel>& CommandHandler::getChannels() const
{
    return channels;
}

/*	Comando /KICK.
	1.-	Se obtiene el nombre de usuario y el nombre del canal.
	2.-	Se llama a la función kickUserFromChannel().
*/
void CommandHandler::handleKickCommand(int client_fd, const std::string& cmdArgs)
{
    size_t spacePosKick = cmdArgs.find(' ');
    std::string userName = cmdArgs.substr(0, spacePosKick);
    std::string channelName = cmdArgs.substr(spacePosKick + 1, cmdArgs.find('\n'));
    kickUserFromChannel(client_fd, userName, channelName);
}

/*	1.-	Primer IF -> Se comprueba si el canal existe.
	2.-	Segundo IF -> Se comprueba si el usuario que envía el comando es el creador del canal.
	3.-	Se recorre el mapa de nicknames para encontrar el usuario a expulsar.
		 -	Si se encuentra, se elimina del canal y se envía un mensaje al usuario expulsado.
		 -	Si no se encuentra, se informa al usuario que el usuario a expulsar no existe.
*/
void CommandHandler::kickUserFromChannel(int client_fd, const std::string& userName, const std::string& channelName)
{
    if (channels.find(channelName) != channels.end())
    {
        if (channels[channelName].creator == client_fd || channels[channelName].operators.count(client_fd))
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

/*	Comando /PRIVMSG.
	1.-	Se obtiene el nombre del usuario o canal y el mensaje.
	2.-	Si el nombre comienza por #, se envía el mensaje a todos los usuarios del canal.
	3.-	Si el nombre no comienza por #, se envía el mensaje al usuario.
*/
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
    std::string formatted_message = "PRIVMSG " + target + " " + sender_user + "!" + sender_nick + " " + message + "\n";

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
    }
    else if (target[0] != '#')
    {
        int target_fd = -1;
        for (std::map<int, std::string>::const_iterator it = nicknames.begin(); it != nicknames.end(); ++it)
        {
            if (it->second == target.substr(1))
			{
				target_fd = it->first;
				break;
			}
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
    }
}

/*	Comando /MODE.
	1.-	Se obtiene el nombre del canal, el modo y el usuario objetivo.
	2.-	Se comprueba si el canal existe.
	3.-	Se comprueba si el usuario objetivo existe.
	4.-	Si el modo es +o, se añade al usuario como operador del canal.
	5.-	Si el modo es -o, se quita al usuario como operador del canal.
*/
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
    for (std::map<int, std::string>::iterator it = nicknames.begin(); it != nicknames.end(); ++it)
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

/*	Comando /INVITE.
	1.-	Se obtiene el nombre del usuario y el nombre del canal.
	2.-	Se comprueba si el canal existe.
	3.-	Se comprueba si el cliente tiene permisos para invitar usuarios al canal.
	4.-	Se busca el fd del usuario objetivo.
	5.-	Si se encuentra, se añade al canal y se envía un mensaje al usuario invitado.
*/
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
    for (std::map<int, std::string>::iterator it = nicknames.begin(); it != nicknames.end(); ++it)
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

/*	Comando /TOPIC.
	1.-	Se obtiene el nombre del canal y el nuevo tema.
	2.-	Se comprueba si el canal existe.
	3.-	Si no se especifica un nuevo tema, se muestra el actual.
	4.-	Se comprueba si el cliente tiene permisos para cambiar el tema.
	5.-	Se establece el nuevo tema y se envía un mensaje a todos los usuarios del canal.
*/
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

/*	Comando WHEREIS.
	1.-	Se obtiene el nombre de usuario o apodo.
	2.-	Se buscan los canales asociados al nombre de usuario y apodo.
	3.-	Si no se encuentran canales, se informa al cliente.
	4.-	Si se encuentran canales, se envía un mensaje al cliente con los nombres de los canales.
*/
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

/*	Comando WHEREAMI.
	1.-	Se obtienen los canales del usuario.
	2.-	Si no hay canales, se informa al cliente.
	3.-	Si hay canales, se envía un mensaje al cliente con los nombres de los canales.
*/
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

/*	Comando /ACTIVE.
	1.-	Se obtiene el nombre del canal.
	2.-	Se comprueba si el usuario está en el canal.
	3.-	Se establece el canal activo y se envía un mensaje al usuario.
*/
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
