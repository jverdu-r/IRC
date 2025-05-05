#include "../includes/event_handler.h"
#include "../includes/socket_manager.h"
#include "../includes/utils.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include "event_handler.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>

EventHandler::EventHandler(SocketManager& socket_manager, CommandHandler& command_handler, UserManager& user_manager,
std::map<int, std::string>& partial_messages, std::map<int, sockaddr_in>& client_addresses, std::set<int>& authenticated_clients) :
    
    authenticated_clients(authenticated_clients),
    partial_messages(partial_messages),
    client_addresses(client_addresses),
	socket_manager(socket_manager),
    command_handler(command_handler),
	user_manager(user_manager)
{
}

EventHandler::~EventHandler()
{
}
 
void EventHandler::handleClientEvent(int client_fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0)
    {
        handleClientDisconnect(client_fd, bytes_received);
    }
    else
    {
        processReceivedData(client_fd, std::string(buffer, bytes_received));
    }
}

void EventHandler::handleClientDisconnect(int client_fd, int bytes_received)
{
    if (bytes_received == 0)
	{
		std::cout << "Cliente " << client_fd << " envió EOF." << std::endl;
	
		if (authenticated_clients.find(client_fd) != authenticated_clients.end())
		{
			std::set<std::string> channels = user_manager.getUserChannels(client_fd);
			if (!channels.empty())
			{
				std::string nickname = socket_manager.getNickname(client_fd);
				std::string disconnectNotice = "Usuario desconectado: " + user_manager.getUserName(client_fd) + " (" + nickname + ")\n";
				for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
				{
					socket_manager.broadcastMessage(disconnectNotice, client_fd, *it);
				}
			}
		}
	}
	else
	{
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        else
        {
		    std::cerr << "Error en recv() para cliente " << client_fd << ": " << strerror(errno) << std::endl;
        }
	}

	epoll_ctl(socket_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
	client_addresses.erase(client_fd);
	partial_messages.erase(client_fd);
}

void EventHandler::processReceivedData(int client_fd, const std::string& received_data)
{
    std::string data = received_data;

    if (partial_messages.find(client_fd) != partial_messages.end())
	{
        data = partial_messages[client_fd] + data;
        partial_messages.erase(client_fd);
    }

    size_t pos = 0;
    while (pos < data.length())
	{
        size_t newline_pos = data.find("\r\n", pos);
        if (newline_pos == std::string::npos)
		{
            newline_pos = data.find('\n', pos);
        }
        if (newline_pos == std::string::npos)
		{
            break;
        }

        std::string line = data.substr(pos, newline_pos - pos);
        pos = newline_pos + ((data[newline_pos] == '\r') ? 2 : 1);

        processLine(client_fd, line);
    }

    if (pos < data.length())
	{
        partial_messages[client_fd] = data.substr(pos);
    }
	else
	{
        partial_messages.erase(client_fd);
    }
}

void EventHandler::processLine(int client_fd, const std::string& line)
{
    if (authenticated_clients.find(client_fd) == authenticated_clients.end() && line.find("/PASS") != 0)
    {
        socket_manager.sendMessageToClient(client_fd, "Por favor autenticate primero.\n");
        return;
    }

    assignDefaultUsername(client_fd);

    if (!line.empty() && line[0] == '/')
    {
        command_handler.handleCommand(client_fd, line);
    }
	else if (!line.empty())
	{
		std::string activeChannel = user_manager.getActiveChannel(client_fd);
        
		if (activeChannel.empty())
		{
			socket_manager.sendMessageToClient(client_fd, "No tienes canal activo. Usa /PRIVMSG <canal> <mensaje>.\n");
		}
		else
		{
			socket_manager.broadcastMessage(line, client_fd, activeChannel);
		}
	}
}

void EventHandler::assignDefaultUsername(int client_fd)
{
    if (user_manager.getUserName(client_fd).empty())
    {
        std::stringstream ss;
        ss << "USER_" << client_fd;
        std::string defaultUsername = ss.str();

        int suffix = 1;
        std::string uniqueUsername = defaultUsername;
        while (user_manager.userNameExists(uniqueUsername))
        {
            ss.str("");
            ss << defaultUsername << suffix;
            uniqueUsername = ss.str();
            suffix++;
        }

        user_manager.setUserName(client_fd, uniqueUsername);
        socket_manager.sendMessageToClient(client_fd, "Nombre de usuario por defecto asignado: " + uniqueUsername + "\n");
    }
}
