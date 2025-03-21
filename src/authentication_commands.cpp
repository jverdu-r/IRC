#include "command_handler.h"
#include <unistd.h>

void CommandHandler::handlePassCommand(int client_fd, const std::string& cmdArgs)
{
    std::string client_password = cmdArgs.substr(0, cmdArgs.find('\n'));
    if (client_password == server_password)
    {
        send(client_fd, "Contraseña correcta. Bienvenido al servidor IRC.\n", 50, 0);
        nicknames[client_fd] = "Invitado";
        authenticated_clients.insert(client_fd);
    }
    else
    {
        send(client_fd, "Contraseña incorrecta. Inténtalo de nuevo.\n", 38, 0);
        close(client_fd);
    }
}

void CommandHandler::handleUserCommand(int client_fd, const std::string& cmdArgs)
{
    if (authenticated_clients.find(client_fd) != authenticated_clients.end())
    {
        std::string username = cmdArgs.substr(0, cmdArgs.find(' '));

        // Verificar unicidad del nombre de usuario
        if (user_manager.userNameExists(username))
        {
            socket_manager.sendMessageToClient(client_fd, "El nombre de usuario ya está en uso.\n");
        }
        else
        {
            // Actualizar mapa de nombres de usuario
            user_manager.setUserName(client_fd, username);
            socket_manager.sendMessageToClient(client_fd, ("Username establecido a " + username + ".\n"));
        }
    }
    else
    {
        socket_manager.sendMessageToClient(client_fd, "Debes autenticarte antes de establecer tu username.\n");
    }
}

void CommandHandler::handleNickCommand(int client_fd, const std::string& cmdArgs)
{
    if (authenticated_clients.find(client_fd) != authenticated_clients.end())
    {
        std::string new_nickname = cmdArgs.substr(0, cmdArgs.find('\n'));
        nicknames[client_fd] = new_nickname;
        socket_manager.sendMessageToClient(client_fd, ("Nickname cambiado a " + new_nickname + ".\n"));
    }
    else
    {
        socket_manager.sendMessageToClient(client_fd, "Debes autenticarte antes de cambiar tu nickname.\n");
    }
}