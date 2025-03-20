#include "../includes/authentication.h"

Authentication::Authentication(const std::string& server_password, std::set<int>& authenticated_clients, std::map<int, std::string>& nicknames, std::map<int, std::string>& userNames)
    : server_password(server_password), authenticated_clients(authenticated_clients), nicknames(nicknames), userNames(userNames) {}

bool Authentication::authenticateUser(int client_fd, const std::string& password) {
    if (password == server_password) {
        authenticated_clients.insert(client_fd);
        nicknames[client_fd] = "Invitado";
        return true;
    }
    return false;
}

bool Authentication::isUserAuthenticated(int client_fd) {
    return authenticated_clients.find(client_fd) != authenticated_clients.end();
}

bool Authentication::setUserNick(int client_fd, const std::string& nick) {
    nicknames[client_fd] = nick;
    return true;
}

bool Authentication::setUserName(int client_fd, const std::string& userName) {
    userNames[client_fd] = userName;
    return true;
}

std::string Authentication::getUserNick(int client_fd) {
    return nicknames[client_fd];
}

std::string Authentication::getUserName(int client_fd) {
    return userNames[client_fd];
}