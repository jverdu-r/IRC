/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket_manager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:11 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/21 16:48:34 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/socket_manager.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring> // para memset
#include <cstdlib> // para exit
#include <cerrno> // Añadido para errno

SocketManager::SocketManager(int port, const std::string& password)
    : server_fd(socket(AF_INET, SOCK_STREAM, 0)),
      epoll_fd(epoll_create1(0)),
      server_password(password),
      client_addresses(), // Inicializar client_addresses
      nicknames(), // Inicializar nicknames
      authenticated_clients(), // Inicializar authenticated_clients
      command_handler(password, nicknames, authenticated_clients, user_manager, *this), // Inicializar command_handler
      user_manager(usernames), // Inicializar user_manager
      usernames(), // Inicializar usernames
      partial_messages(), // Inicializar partial_messages
      event_handler(*this, command_handler, user_manager, partial_messages, client_addresses, authenticated_clients) // Inicializar event_handler
{
    // 1. Crear el socket del servidor
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

    // 2. Configurar la direccion del servidor
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // 3. Asociar el socket a la direccion del servidor
    if (bind(server_fd, (sockaddr*)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "Error al asociar el socket a la direccion." << std::endl;
        exit(1);
    }

    // 4. Poner el socket en modo de escucha
    if (listen(server_fd, 5) == -1)
    {
        std::cerr << "Error al poner el socket en modo escucha." << std::endl;
        exit(1);
    }

    // 5. Crear la instancia de epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cerr << "Error al crear la instancia de epoll." << std::endl;
        exit(1);
    }

    // 6. Agregar el socket del servidor a epoll
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

    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
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
                // Cliente desconectado o error
                //std::cout << "Cliente " << events[i].data.fd << " desconectado o error." << std::endl;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
                client_addresses.erase(events[i].data.fd);
                partial_messages.erase(events[i].data.fd);
            }
        }
    }
}

void SocketManager::acceptConnection()
{
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
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
    nicknames[client_fd] = "Invitado"; // Inicializar el nickname aquí
    //std::cout << "Nuevo cliente conectado: " << client_fd << std::endl;
    // Enviar mensaje de bienvenida
    std::string welcome_message = "Bienvenido al servidor IRC. Por favor, introduce la contraseña con el comando PASS.\n";
    send(client_fd, welcome_message.c_str(), welcome_message.length(), 0);
}

void SocketManager::broadcastMessage(const std::string& message, int sender_fd, const std::string& channelName) {
    std::string sender_nickname = nicknames[sender_fd];
    std::string sender_username = user_manager.getUserName(sender_fd);
    std::string formatted_message = "[" + sender_username + "!" + sender_nickname + "]: " + message + '\n';
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

void SocketManager::sendMessageToClient(int client_fd, const std::string& message)
{
    //std::cout << "Enviando mensaje al cliente " << client_fd << ": " << message;
    send(client_fd, message.c_str(), message.length(), 0);
}