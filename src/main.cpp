/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jverdu-r <jverdu-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 16:01:06 by jverdu-r          #+#    #+#             */
/*   Updated: 2025/03/20 17:50:06 by jverdu-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/socket_manager.h"
#include "../includes/command_handler.h"
#include "../includes/authentication.h"
#include "../includes/user_manager.h"
#include <iostream>
#include <cstdlib>
#include <map>
#include <set>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <puerto> <contraseña>" << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Puerto invalido. Debe estar entre 1 y 65535." << std::endl;
        return 1;
    }

    std::string server_password = argv[2];

    std::map<int, std::string> nicknames;
    std::set<int> authenticated_clients;
    std::map<int, std::string> userNames;
    UserManager user_manager(userNames); // Pass userNames map to UserManager constructor

    Authentication authentication(server_password, authenticated_clients, nicknames, userNames);
    SocketManager server(port, server_password, nicknames, authenticated_clients, user_manager, authentication);

    server.run();
    return 0;
}