#include "command_handler.h"
#include <unistd.h>

/*	Gestiona el comando /PASS.
	1.-	Se extrae la contraseña del mensaje.
	2.-	Se comprueba si la contraseña es correcta, y se envía un mensaje al cliente según el resultado.
		Se cierra la conexión si la contraseña es incorrecta.
*/
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

/*	Se gestiona el comando /NICK, es decirt, la solicitud de cambio de apodo de usuario.
	1.-	Se extrae el apodo del mensaje.
	2.-	Se comprueba si el apodo ya está en uso, y se envía un mensaje al cliente
		según el resultado.
*/
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

/*	Se gestiona el comando /USER, es decir, la solicitud de cambio de nombre de usuario.
	1.-	Se extrae el nombre de usuario del mensaje.
	2.-	Se comprueba si el nombre de usuario ya está en uso, y se envía un mensaje al cliente
		según el resultado.
*/
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
