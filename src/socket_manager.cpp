/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket_manager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:11 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/08 17:46:50 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/socket_manager.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring> // para memset
#include <cstdlib> // para exit
#include <cerrno> // Añadido para errno

/*	En la construcción se inicializa:
		- client_addresses
		- nicknames
		- authenticated_clients
		- command_handler
		- user_manager
		- usernames
		- partial_messages
		- event_handler
	En el cuerpo del constructor se siguen los siguientes pasos:
		1 -	Se crea el socket del servidor:
				Activamos la opción SO_REUSEADDR para permitir reutilizar el puerto inmediatamente después de
				cerrar el servidor, evitando errores como "Address already in use". Es útil especialmente durante
				el desarrollo o reinicios frecuentes del servidor, ya que la espero puede ser de 1-4 minutos.
		2 -	Configurar la direccion del servidor: se prepara una estructura para que el socket escuche en cualquier
			IP (0.0.0.0) y el puerto indicado.
		3 -	Asociar el socket a la direccion del servidor; une el socket a la IP y puerto configurados.
		4 -	Poner el socket en modo de escucha.
				Gracias a (bind + listen) sabemos que entran solo las conexiones dirigidas a este programa.
		5 -	Crear la instancia de epoll.
		6 -	Agregar el socket del servidor a epoll para que el programa sea notificado cuando lleguen nuevas
			conexiones entrantes.
				-	Con:
						event.data.fd = server_fd;
					se configura ese evento para que escuche eventos de lectura (EPOLLIN) en el descriptor server_fd.
				-	Se utiliza epoll_ctl para registrar ese socket en el epoll.
*/
SocketManager::SocketManager(int port, const std::string& password) :
	server_password(password),
    authenticated_clients(),
    client_addresses(),
    nicknames(),
    usernames(),
    partial_messages(),
    user_manager(usernames, *this),
    event_handler(*this, command_handler, user_manager, partial_messages, client_addresses, authenticated_clients),
    command_handler(password, nicknames, authenticated_clients, user_manager, *this)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Error al crear el socket del servidor." << std::endl;
        exit(1);
    }
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        std::cerr << "Error al configurar SO_REUSEADDR." << std::endl;
        exit(1);
    }

    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "Error al asociar el socket a la direccion." << std::endl;
        exit(1);
    }

    if (listen(server_fd, 5) == -1)
    {
        std::cerr << "Error al poner el socket en modo escucha." << std::endl;
        exit(1);
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cerr << "Error al crear la instancia de epoll." << std::endl;
        exit(1);
    }

    epoll_event	event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        std::cerr << "Error al agregar el socket del servidor a epoll." << std::endl;
        exit(1);
    }

    std::cout << "Servidor escuchando en el puerto " << port << "..." << std::endl;
}

SocketManager::~SocketManager()
{
    close(epoll_fd);
    close(server_fd);
}

/*	Este es el método llamado desde main() que hace "funcionar" el servidor.
	1.-	Reservamos un array donde epoll depositará los eventos que detecte.
	2.- Iniciamos el bucle infinito:
		  -	Utilizamos epoll_wait con la opción -1 que indica que epoll_wait es bloqueante, es decir,
			espera indefinidamente.
		  -	Solo si epoll_wait devuelve -1 (por un error) se termina el bucle.
		  -	Si se reciben eventos:
			  -	Si el evento es EPOLLIN:
				  -	Si es el socket del servidor, es decir, si es server_fd, significa un cliente intenta
				  	conectarse, y se llama al método que gestiona la conexión.
				  -	Si no, si el evento viene de un socket de cliente, se delega el procesamiento al manejador
				  	de eventos.
			  -	Si no, si el evento es una desconexión (EPOLLHUP) o un error (EPOLLERR), hay que dejar todo limpio:
			  	  -	Se elimina el cliente del epoll.
				  -	Se cierra el socket del cliente.
				  -	Se eliminan sus datos de los mapas de IPs.
				  -	Se eliminan sus mensajes parciales.
*/
void SocketManager::run()
{
    epoll_event events[MAX_EVENTS];

    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
            std::cerr << "Error en epoll_wait: " << strerror(errno) << std::endl;
            break;
        }

        for (int i = 0; i < num_events; ++i)
        {
            if (events[i].events & EPOLLIN)
            {
                if (events[i].data.fd == server_fd)
                {
                    acceptConnection();
                }
                else
                {
                    event_handler.handleClientEvent(events[i].data.fd);
                }
            }
            else if (events[i].events & (EPOLLHUP | EPOLLERR))
            {
                std::cout << "Cliente " << events[i].data.fd << " desconectado o error." << std::endl;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
                client_addresses.erase(events[i].data.fd);
                partial_messages.erase(events[i].data.fd);
            }
        }
    }
}

/*	Se encarga de aceptar una conexión entrante; se utiliza una estructura sockaddr_in (y su tamaño, que se
	necesita para accept()) para almacenar la dirección IP y puerto del cliente que se conectará.
	  -	accept() espera una conexión entrante en el server_fd, al llegar crea un nuevo socket (client_fd)
	  	exclusivo para la comunicación con ese cliente.
	  - Utilizamos fcntl() para que las operaciones de lectura/escritura sobre el socket no bloqueen el hilo.
	  	Importante para servidores con Epoll, donde no queremos que un cliente lento bloquee a todos.
	  -	Se crea un evento epoll para ese socket de cliente, se configura para escuchar eventos de entrada (EPOLLIN).
	  -	Se registra en el epoll_fd con epoll_ctl() para que el servidor sea notificado cuando ese cliente envíe
	  	datos.
	  -	Finalmente se almacena la dirección IP y puerto del cliente en el mapa de direcciones, se le asigna un
	  	nickname por defecto y se envía un mensaje de bienvenida.
*/
void SocketManager::acceptConnection()
{
    sockaddr_in	client_address;
    socklen_t	client_address_len = sizeof(client_address);
    int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_address_len);

    if (client_fd < 0)
    {
        std::cerr << "Error en accept(): " << strerror(errno) << std::endl;
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
	
    client_addresses[client_fd] = client_address;
    nicknames[client_fd] = "Invitado";
    std::cout << "Nuevo cliente conectado: " << client_fd << std::endl;
    std::string welcome_message = "Bienvenido al servidor IRC. Por favor, introduce la contraseña con el comando /PASS.\n";	// Se crea el mensaje de bienvenida
    send(client_fd, welcome_message.c_str(), welcome_message.length(), 0);
}

/*	Se encarga de enviar un mensaje a todos los clientes autenticados excepto al que lo envía.
	1.-	Se obtienen los datos del emisor, se formatea el mensaje y se guardan todos los canales.
	2.-	Si el canal existe, se envía el mensaje a todos los usuarios del canal excepto al emisor.
		La lista de usuarios del canal se recupera como un std::set<int> (cada usuario está representado
		por su socket fd).
*/
void SocketManager::broadcastMessage(const std::string& message, int sender_fd, const std::string& channelName)
{
    std::string	sender_nickname = nicknames[sender_fd];
    std::string	sender_username = user_manager.getUserName(sender_fd);
    std::string formatted_message = "#" + channelName + " -> " + sender_username + "!" + sender_nickname + ": " + message + '\n';
    const std::map<std::string, Channel>& channels = command_handler.getChannels();
	
    if (channels.find(channelName) != channels.end())
    {
        std::set<int>::iterator it;
        for (it = channels.find(channelName)->second.users.begin(); it != channels.find(channelName)->second.users.end(); ++it)
        {
            int userFd = *it;
            if (userFd != sender_fd && authenticated_clients.find(userFd) != authenticated_clients.end())
            {
                send(userFd, formatted_message.c_str(), formatted_message.length(), 0);
            }
        }
    }
}

/*	Sencillo método para enviar un mensaje a un cliente.
	Se utiliza send() con el descriptor del cliente, el mensaje y su longitud.
*/

void	SocketManager::sendMessageToClient(int client_fd, const std::string& message)
{
    std::string	sender_nickname = nicknames[client_fd];
    std::string	sender_username = user_manager.getUserName(client_fd);
    std::string formatted_message = sender_username + "!" + sender_nickname + ": " + message + '\n';
	
    send(client_fd, message.c_str(), message.length(), 0);
}

/*	Método para obtener el descriptor del epoll.
*/

int		SocketManager::getEpollFd()
{ 
	return epoll_fd;
}

/*	Devuelve el apodo de un cliente a partir de su descriptor.
*/
std::string SocketManager::getNickname(int client_fd) const
{
    std::map<int, std::string>::const_iterator it = nicknames.find(client_fd);
    if (it != nicknames.end())
    {
        return it->second;
    }
    return "";
}

/*	Devuelve el mapa de apodos.
*/
const std::map<int, std::string>& SocketManager::getNicknames() const
{
    return this->nicknames;
}
