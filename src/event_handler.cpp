#include "../includes/event_handler.h"
#include "../includes/socket_manager.h"
#include "../includes/utils.h"
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

/*	Se encarga de gestionar los eventos de los clientes.
	1.-	Se crea un buffer de 1024 bytes y se inicializa a 0.
	2.-	Se recibe el mensaje del cliente y se guarda en bytes_received.
	3.-	Si bytes_received es menor o igual a 0, se llama a handleClientDisconnect().
	4.-	Si bytes_received es mayor que 0, se llama a processReceivedData().
*/  
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

/*	Se encarga de gestionar la desconexión de un cliente.
	 -	Si bytes_received es 0, se informa de que el cliente envió EOF.
		Si el cliente está autenticado, se obtiene el canal del usuario y se envía el mensaje
		parcial a todos los usuarios del canal.
	 -	Si bytes_received es distinto de 0, se informa del error en recv().
	En cualquier caso:
	 -	Se elimina el cliente del epoll.
	 -	Se cierra el socket del cliente.
	 -	Se eliminan los datos del cliente de los mapas de IPs.
	 -	Se eliminan los mensajes parciales del cliente.	
*/
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
		std::cerr << "Error en recv() para cliente " << client_fd << ": " << strerror(errno) << std::endl;
	}

	epoll_ctl(socket_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
	client_addresses.erase(client_fd);
	partial_messages.erase(client_fd);
}

/*	Se encarga de procesar los datos recibidos.
	1.-	Si el cliente tiene mensajes parciales, se concatenan con los datos recibidos.
	2.-	Se recorre la cadena de datos recibidos y se procesa línea a línea.
	3.-	Si hay mensajes parciales (tras procesar las líneas, la última puede ser incompleta, por 
		ejemplo), se guardan en el mapa de mensajes parciales.
*/
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

/*	Se encarga de procesar una línea de datos.
	1.-	Si el cliente no está autenticado y la línea no comienza con /PASS, se informa al cliente.
	2.-	Se asigna un nombre de usuario por defecto si no tiene uno.
	3.-	Si la línea comienza con /, se procesa como un comando.
	4.-	Si no, se envía el mensaje a todos los usuarios del canal del cliente.
*/
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

/*	Se encarga de asignar un nombre de usuario por defecto al cliente.
	1.-	Se obtiene el nombre de usuario del cliente.
	2.-	Si el nombre de usuario está vacío, se asigna un nombre de usuario por defecto.
	3.-	Se comprueba la unicidad del nombre de usuario.
	4.-	Se asigna el nombre de usuario y se informa al cliente del nombre de usuario asignado.
*/
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
