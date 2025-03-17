#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <set>
#include <string>

#define MAX_EVENTS 10 //numero de evento que epoll puede manejar a la vez

class SocketManager 
{
	public:
		SocketManager(int port, const std::string& password);
		~SocketManager();
		void run();
	private:
		int server_fd;
		int epoll_fd;
		std::map<int, std::string> nicknames;
		std::map<int, std::string> partial_messages;
		std::map<int, sockaddr_in> client_addresses;
		std::set<int> authenticated_clients;
		std::string server_password;
		void acceptConnection();
		void handleClientEvent(int client_fd);
		void broadcastMessage(const std::string& message, int sender_fd);
};

#endif