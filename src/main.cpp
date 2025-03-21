/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:06 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/21 16:45:30 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/socket_manager.h"
#include "../includes/command_handler.h"
#include <iostream>
#include <cstdlib>

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

	std::string password = argv[2];

	SocketManager server(port, password);
	server.run();
	return (0);
}