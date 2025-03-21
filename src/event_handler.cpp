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
                if (!channel.empty()) {
                    socket_manager.broadcastMessage(partial_messages[client_fd], client_fd, channel);
                }
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

            // Divide la entrada en líneas separadas
            size_t start_pos = 0;
            while (start_pos < received_data.length()) {
                size_t end_pos = received_data.find('\n', start_pos);
                if (end_pos == std::string::npos) {
                    break;
                }
                std::string line = received_data.substr(start_pos, end_pos - start_pos);
                start_pos = end_pos + 1;

                // Verifica la autenticación antes de procesar la línea
                if (authenticated_clients.find(client_fd) == authenticated_clients.end() && line.find("/PASS") != 0) {
                    socket_manager.sendMessageToClient(client_fd, "Por favor autenticate primero.\n");
                    continue; // Salta al siguiente comando
                }

                // Procesa comandos y mensajes regulares
                if (!line.empty() && line[0] == '/') {
                    std::cout << "Comando recibido: " << line << std::endl;
                    command_handler.handleCommand(client_fd, line);
                } else if (!line.empty()){
                    // Manejar mensajes regulares
                    std::string channel = user_manager.getUserChannel(client_fd);

                    // Verifica si el usuario está en algún canal
                    if (channel.empty()) {
                        socket_manager.sendMessageToClient(client_fd, "No puedes enviar mensajes si no estás en un canal.\n");
                    } else {
                        std::cout << "Canal del usuario " << client_fd << ": " << channel << std::endl;
                        socket_manager.broadcastMessage(line, client_fd, channel);
                    }
                }
            }
        } else {
            partial_messages[client_fd] = received_data;
        }
    }
}