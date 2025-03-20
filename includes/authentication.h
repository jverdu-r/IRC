#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <string>
#include <set>
#include <map>

class Authentication {
public:
    Authentication(const std::string& server_password, std::set<int>& authenticated_clients, std::map<int, std::string>& nicknames, std::map<int, std::string>& userNames);
    bool authenticateUser(int client_fd, const std::string& password);
    bool isUserAuthenticated(int client_fd);
    bool setUserNick(int client_fd, const std::string& nick);
    bool setUserName(int client_fd, const std::string& userName);
    std::string getUserNick(int client_fd);
    std::string getUserName(int client_fd);

private:
    std::string server_password;
    std::set<int>& authenticated_clients;
    std::map<int, std::string>& nicknames;
    std::map<int, std::string>& userNames;
};

#endif