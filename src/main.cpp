/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:06 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/04/08 17:47:14 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/socket_manager.h"
#include "../includes/command_handler.h"
#include <iostream>
#include <cstdlib>

#ifdef BONUS_MODE
void launchBot();

void* botThread(void*)
{
    launchBot();
    return NULL;
}
#endif


int main(int argc, char* argv[])
{
	if (argc != 3)
	{
        std::cerr << "Uso: " << argv[0] << " <puerto> <contraseña>" << std::endl;
        return 1;
    }
	int port = atoi(argv[1]);
	if (port <= 0 || port > 65535)
	{
        std::cerr << "Puerto invalido. Debe estar entre 1 y 65535." << std::endl;
        return 1;
    }
	
	#ifdef BONUS_MODE
	pthread_t thread;
	pthread_create(&thread, NULL, botThread, NULL);
	pthread_detach(thread);
	#endif

	std::string password = argv[2];

	SocketManager server(port, password);
	server.run();
	return (0);
}