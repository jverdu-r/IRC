#include "../includes/user_manager.h"

UserManager::UserManager(std::map<int, std::string>& usernames) : usernames(usernames) {}

void UserManager::setUserName(int client_fd, const std::string& username)
{
	usernames[client_fd] = username;
}

std::string UserManager::getUserName(int client_fd)
{
	if (usernames.find(client_fd) != usernames.end())
	{
		return usernames[client_fd];
	}
	return "";
}