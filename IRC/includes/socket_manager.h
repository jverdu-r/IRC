#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <string>

#define MAX_EVENTS 10 //numero de evento que epoll puede manejar a la vez

class SocketManager 
{
	private:
		int server_fd;
		int epoll_fd;
		std::map<int, std::string> partial_messages;
		std::map<int, sockaddr_in> client_addresses;
		void acceptConnection();
		void handleClientEvent(int client_fd);
		void broadcastMessage(const std::string& message, int sender_fd);
	public:
		SocketManager(int port);
		~SocketManager();
		void run();
};

#endif