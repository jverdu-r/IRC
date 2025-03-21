#include "../includes/event_handler.h"
#include "../includes/socket_manager.h"
#include <iostream>
#include <cstring> // Para memset
#include <unistd.h> // Para recv, close
#include <cerrno> // Para errno
#include <sstream>
#include "event_handler.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>


EventHandler::EventHandler(SocketManager& socket_manager,
    CommandHandler& command_handler,
    UserManager& user_manager,
    std::map<int, std::string>& partial_messages,
    std::map<int, sockaddr_in>& client_addresses,
    std::set<int>& authenticated_clients)
    : socket_manager(socket_manager),
    command_handler(command_handler), user_manager(user_manager),
    partial_messages(partial_messages),
    client_addresses(client_addresses),
    authenticated_clients(authenticated_clients) {}

   
    
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
        //std::cout << "Cliente " << client_fd << " envió EOF." << std::endl;
        if (partial_messages.find(client_fd) != partial_messages.end())
        {
            //std::cout << "Datos recibidos del cliente " << client_fd << ": " << partial_messages[client_fd];
            if (authenticated_clients.find(client_fd) != authenticated_clients.end())
            {
                std::string channel = user_manager.getUserChannel(client_fd);
                if (!channel.empty())
                {
                    socket_manager.broadcastMessage(partial_messages[client_fd], client_fd, channel);
                }
            }
            partial_messages.erase(client_fd);
        }
        else
        {
            epoll_ctl(socket_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
            close(client_fd);
            client_addresses.erase(client_fd);
        }
    }
    else
    {
        std::cerr << "Error en recv() para cliente " << client_fd << ": " << strerror(errno) << std::endl;
        epoll_ctl(socket_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
        close(client_fd);
        client_addresses.erase(client_fd);
        partial_messages.erase(client_fd);
    }
}

void EventHandler::processReceivedData(int client_fd, const std::string& received_data)
{
    std::string data = received_data;

    if (partial_messages.find(client_fd) != partial_messages.end())
    {
        data = partial_messages[client_fd] + data;
        partial_messages.erase(client_fd);
    }

    size_t newline_pos = data.find('\n');

    if (newline_pos != std::string::npos)
    {
        //std::cout << "Datos recibidos del cliente " << client_fd << ": " << data;
        size_t start_pos = 0;
        while (start_pos < data.length())
        {
            size_t end_pos = data.find('\n', start_pos);
            if (end_pos == std::string::npos)
            {
                break;
            }
            std::string line = data.substr(start_pos, end_pos - start_pos);
            start_pos = end_pos + 1;

            processLine(client_fd, line);
        }
    }
    else
    {
        partial_messages[client_fd] = data;
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
        //std::cout << "Comando recibido: " << line << std::endl;
        command_handler.handleCommand(client_fd, line);
    }
    else if (!line.empty())
    {
        std::string channel = user_manager.getUserChannel(client_fd);
        if (channel.empty())
        {
            socket_manager.sendMessageToClient(client_fd, "No puedes enviar mensajes si no estás en un canal.\n");
        }
        else
        {
            //std::cout << "Canal del usuario " << client_fd << ": " << channel << std::endl;
            socket_manager.broadcastMessage(line, client_fd, channel);
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