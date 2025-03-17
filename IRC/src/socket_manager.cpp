#include "../includes/socket_manager.h"
#include <iostream>
#include <cstring> // para memset
#include <cstdlib> // para exit

SocketManager::SocketManager(int port)
{
	// 1.Crear el socket del servidor
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << "Error al crear el socket del servidor." << std::endl;
		exit(1);
	}

	// 2.Configurar la direccion del servidor
	sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	// 3.Asociar el socket a la direccion del servidor
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
		std::cerr << "Error al configurar SO_REUSEADDR." << std::endl;
		exit(1);
	}
	if (bind(server_fd, (sockaddr*)&server_address, sizeof(server_address)) == -1)
	{
		std::cerr << "Error al asociar el socket a la direccion." << std::endl;
		exit(1);
	}

	// 4.Poner el socket en modo de escucha
	if (listen(server_fd, 5) == -1)
	{
		std::cerr << "Error al poner el socket en modo escucha." << std::endl;
		exit(1);
	}

	// 5.Crear la instancia de epoll
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << "Error al crear la instancia de epoll." << std::endl;
		exit(1);
	}

	// 6.Agregar el socket del servidor a epoll
	epoll_event event;
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

void SocketManager::run()
{
	epoll_event events[MAX_EVENTS];
	while(true)
	{
		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (num_events == -1)
		{
			std::cerr << "Error en epoll_wait." << std::endl;
			continue;
		}
		for (int i = 0; i < num_events; ++i)
		{
			if (events[i].data.fd == server_fd)
			{
				acceptConnection();
			}
			else
			{
				handleClientEvent(events[i].data.fd);
			}
		}
	}
}

void SocketManager::acceptConnection()
{
	sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);
	int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_address_len);
	if (client_fd == -1)
	{
		std::cerr << "Error al aceptar la conexion." << std::endl;
		return;
	}

	// Configurar el socket del cliente como no bloqueante
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Agregar el socket del cliente a epoll
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
	{
		std::cerr << "Error al agragar el socket del cliente a epoll." << std::endl;
		close(client_fd);
		return;
	}

	client_addresses[client_fd] = client_address;
	std::cout << "Nuevo cliente conectado: " << client_fd << std::endl;
}

#include <cstring> // Para strerror
#include <errno.h> // Para errno

void SocketManager::handleClientEvent(int client_fd) { //problema persistente con "ctrl+d" cuando se ejecuta multiples veces en una linea o en una linea vacia
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    //std::cout << "bytes_received: " << bytes_received << std::endl;
    //std::cout << "buffer: " << buffer << std::endl;

    if (bytes_received == 0)
	{
        // Cliente envió EOF (Ctrl+D)
        std::cout << "Cliente " << client_fd << " envió EOF." << std::endl;
        if (partial_messages.find(client_fd) != partial_messages.end())
		{
            std::cout << "Datos recibidos del cliente " << client_fd << ": " << partial_messages[client_fd];
			broadcastMessage(partial_messages[client_fd], client_fd);
			//send(client_fd, "Mensaje recibido.\n", 18, 0);
            partial_messages.erase(client_fd);
        }
		else
		{
            // No hay mensajes parciales, desconectar
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
            close(client_fd);
            client_addresses.erase(client_fd);
        }
    }
	else if (bytes_received < 0)
	{
        std::cerr << "Error en recv() para cliente " << client_fd << ": " << strerror(errno) << std::endl;
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        close(client_fd);
        client_addresses.erase(client_fd);
        partial_messages.erase(client_fd);
    }
	else
	{
        std::string received_data(buffer, bytes_received);
        //std::cout << "received_data: " << received_data << std::endl;

        if (partial_messages.find(client_fd) != partial_messages.end())
		{
            //std::cout << "partial_messages: " << partial_messages[client_fd] << std::endl;
            received_data = partial_messages[client_fd] + received_data;
            partial_messages.erase(client_fd);
        }

        size_t newline_pos = received_data.find('\n');
        //std::cout << "newline_pos: " << newline_pos << std::endl;

        if (newline_pos != std::string::npos)
		{
            std::cout << "Datos recibidos del cliente " << client_fd << ": " << received_data;
            broadcastMessage(received_data, client_fd);
			//send(client_fd, "Mensaje recibido.\n", 18, 0);
        } else {
            partial_messages[client_fd] = received_data;
        }
    }
}

void SocketManager::broadcastMessage(const std::string& message, int sender_fd)
{
	for (std::map<int, sockaddr_in>::iterator it = client_addresses.begin(); it != client_addresses.end(); ++it)
	{
		int client_fd = it->first;
		if (client_fd != sender_fd)
		{
			send(client_fd, message.c_str(), message.length(), 0);
		}
	}
}