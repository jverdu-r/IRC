#include "../includes/socket_manager.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[])
{
	if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>" << std::endl;
        return 1;
    }
	int port = atoi(argv[1]);
	if (port <= 0 || port > 65535) {
        std::cerr << "Puerto invalido. Debe estar entre 1 y 65535." << std::endl;
        return 1;
    }
	SocketManager server(port);
	server.run();
	return (0);
}