#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <map>

class UserManager
{
	public:
		UserManager(std::map<int, std::string>& usernames);
		void setUserName(int client_fd, const std::string& username);
		std::string getUserName(int client_fd);
	private:
		std::map<int, std::string>& usernames;
};

#endif