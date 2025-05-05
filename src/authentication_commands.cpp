#include "command_handler.h"
#include "socket_manager.h"
#include <unistd.h>
#include <string.h>

void CommandHandler::handlePassCommand(int client_fd, const std::string& cmdArgs)
{
    std::string client_password = cmdArgs.substr(0, cmdArgs.find('\n'));
    if (client_password == server_password)
    {
        send(client_fd, " \n", strlen(" \n"), 0);
        send(client_fd, "Contraseña correcta!!\n", strlen("Contraseña correcta!!\n"), 0);
		authenticated_clients.insert(client_fd);
		nicknames[client_fd] = "Invitado";
        send(client_fd, " \n", strlen(" \n"), 0);
        send(client_fd, "Bienvenido al servidor IRC!! :)\n", strlen("Bienvenido al servidor IRC!! :)\n"), 0);
        send(client_fd, " \n", strlen(" \n"), 0);
		send(client_fd, "Aquí tienes los comandos posibles:\n", strlen("Aquí tienes los comandos posibles:\n"), 0);
        send(client_fd, " \n", strlen(" \n"), 0);
		send(client_fd, "/PASS\t\t->\tintroduce el password\n", strlen("/PASS\t\t->\tintroduce el password\n"), 0);
		send(client_fd, "/NICK\t\t->\tpara cambiar tu apodo\n", strlen("/NICK\t\t->\tpara cambiar tu apodo\n"), 0);
		send(client_fd, "/USER\t\t->\tpara cambiar tu usuario\n", strlen("/USER\t\t->\tpara cambiar tu usuario\n"), 0);
		send(client_fd, "/JOIN\t\t->\tpara unirte a un canal\n", strlen("/JOIN\t\t->\tpara unirte a un canal\n"), 0);
		send(client_fd, "/PART\t\t->\tpara salir de un canal\n", strlen("/PART\t\t->\tpara salir de un canal\n"), 0);
		send(client_fd, "/KICK\t\t->\tpara echar a un usuario de un canal\n", strlen("/KICK\t\t->\tpara echar a un usuario de un canal\n"), 0);
		send(client_fd, "/NAMES\t\t->\tlista los usuarios de un canal\n", strlen("/NAMES\t\t->\tlista los usuarios de un canal\n"), 0);
		send(client_fd, "/LIST\t\t->\tlista todos los canales\n", strlen("/LIST\t\t->\tlista todos los canales\n"), 0);
		send(client_fd, "/PRIVMSG\t->\tenvia un mensaje privado a un canal o usuario\n", strlen("/PRIVMSG\t->\tenvia un mensaje privado a un canal o usuario\n"), 0);
		send(client_fd, "/MODE\t\t->\tda (+o) o retira (-o) permisos de operador\n", strlen("/MODE\t\t->\tda (+o) o retira (-o) permisos de operador\n"), 0);
		send(client_fd, "/INVITE\t\t->\tinvita a un usuario al canal\n", strlen("/INVITE\t\t->\tinvita a un usuario al canal\n"), 0);
		send(client_fd, "/TOPIC\t\t->\tcambia la descripción de un canal \n", strlen("/TOPIC\t\t->\tcambia la descripción de un canal \n"), 0);
		send(client_fd, "/WHEREIS\t->\tlista los canales en los que está un usuario\n", strlen("/WHEREIS\t->\tlista los canales en los que está un usuario\n"), 0);
		send(client_fd, "/WHEREAMI\t->\tlista los canales en los que estás\n", strlen("/WHEREAMI\t->\tlista los canales en los que estás\n"), 0);
		send(client_fd, "/ACTIVE\t\t->\tcambia tu canal activo (en el que escribes)\n", strlen("/ACTIVE\t\t->\tcambia tu canal activo (en el que escribes)\n"), 0);
        send(client_fd, "/HELP\t\t->\tmuestra la tabla de comandos\n", strlen("/HELP\t\t->\tmuestra la tabla de comandos\n"), 0);
        send(client_fd, " \n", strlen(" \n"), 0);
    }
    else
    {
        send(client_fd, "Contraseña incorrecta. Inténtalo de nuevo.\n", 46, 0);
        close(client_fd);
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

void CommandHandler::handleUserCommand(int client_fd, const std::string& cmdArgs)
{
    if (authenticated_clients.find(client_fd) != authenticated_clients.end())
    {
        std::string username = cmdArgs.substr(0, cmdArgs.find(' '));

        if (user_manager.userNameExists(username))
        {
            socket_manager.sendMessageToClient(client_fd, "El nombre de usuario ya está en uso.\n");
        }
        else
        {
            user_manager.setUserName(client_fd, username);
            socket_manager.sendMessageToClient(client_fd, ("Username establecido a " + username + ".\n"));
        }
    }
    else
    {
        socket_manager.sendMessageToClient(client_fd, "Debes autenticarte antes de establecer tu username.\n");
    }
}
