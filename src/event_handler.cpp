#include "../includes/event_handler.h"
#include "../includes/socket_manager.h"
#include <iostream>
#include <cstring> // Para memset
#include <unistd.h> // Para recv, close
#include <cerrno> // Para errno


EventHandler::EventHandler(SocketManager& socket_manager, CommandHandler& command_handler, UserManager& user_manager, std::map<int, std::string>& partial_messages, std::map<int, sockaddr_in>& client_addresses, std::set<int>& authenticated_clients)
    : socket_manager(socket_manager), command_handler(command_handler), user_manager(user_manager), partial_messages(partial_messages), client_addresses(client_addresses), authenticated_clients(authenticated_clients) {}

    void EventHandler::handleClientEvent(int client_fd) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
        if (bytes_received == 0) {
            // Cliente envió EOF (Ctrl+D)
            std::cout << "Cliente " << client_fd << " envió EOF." << std::endl;
            if (partial_messages.find(client_fd) != partial_messages.end()) {
                std::cout << "Datos recibidos del cliente " << client_fd << ": " << partial_messages[client_fd];
                if (authenticated_clients.find(client_fd) != authenticated_clients.end()) {
                    std::string channel = user_manager.getUserChannel(client_fd);
                    socket_manager.broadcastMessage(partial_messages[client_fd], client_fd, channel);
                }
                partial_messages.erase(client_fd);
            } else {
                // No hay mensajes parciales, desconectar
                epoll_ctl(socket_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
                close(client_fd);
                client_addresses.erase(client_fd);
            }
        } else if (bytes_received < 0) {
            std::cerr << "Error en recv() para cliente " << client_fd << ": " << strerror(errno) << std::endl;
            epoll_ctl(socket_manager.getEpollFd(), EPOLL_CTL_DEL, client_fd, NULL);
            close(client_fd);
            client_addresses.erase(client_fd);
            partial_messages.erase(client_fd);
        } else {
            std::string received_data(buffer, bytes_received);
    
            if (partial_messages.find(client_fd) != partial_messages.end()) {
                received_data = partial_messages[client_fd] + received_data;
                partial_messages.erase(client_fd);
            }
    
            size_t newline_pos = received_data.find('\n');
    
            if (newline_pos != std::string::npos) {
                std::cout << "Datos recibidos del cliente " << client_fd << ": " << received_data;
    
                // Enviar todos los comandos a command_handler
                if (received_data.find('/') == 0 || received_data.find("NICK ") == 0 || received_data.find("USER ") == 0 || received_data.find("PASS ") == 0) {
                    std::cout << "Comando recibido: " << received_data << std::endl;
                    command_handler.handleCommand(client_fd, received_data);
                } else {
                    // Manejar mensajes regulares
                    if (authenticated_clients.find(client_fd) != authenticated_clients.end()) {
                        std::string channel = user_manager.getUserChannel(client_fd);
                        std::cout << "Canal del usuario " << client_fd << ": " << channel << std::endl;
                        socket_manager.broadcastMessage(received_data, client_fd, channel);
                    } else {
                        socket_manager.sendMessageToClient(client_fd, "Por favor autenticate primero.\n");
                    }
                }
            } else {
                partial_messages[client_fd] = received_data;
            }
        }
    }