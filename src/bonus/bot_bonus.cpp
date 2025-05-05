/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot_bonus.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolopez- <jolopez-@student.42madrid>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/05 00:00:11 by jolopez-          #+#    #+#             */
/*   Updated: 2025/04/08 19:07:44 by jolopez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <cctype>

int 						sock = 0;
std::vector<std::string>	known_channels;
std::string					bot_nick = "HAL9000 🤖";
volatile bool 				running = true;

void sendMessage(const std::string& message)
{
	std::string formatted = message + "\r\n";
	send(sock, formatted.c_str(), formatted.length(), 0);
}

std::string cleanMessage(const std::string& msg)
{
	size_t first = msg.find_first_not_of(" \n\r\t");
	size_t last = msg.find_last_not_of(" \n\r\t");
	if (first == std::string::npos || last == std::string::npos)
		return "";
	return msg.substr(first, (last - first + 1));
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);
	return tokens;
}

std::string getCurrentTime()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%H:%M:%S", &tstruct);
	return std::string(buf);
}

void signalHandler(int signum)
{
    std::cout << "\n[Bot] Señal de interrupción (" << signum << ") recibida. Cerrando bot..." << std::endl;
    running = false;
    close(sock);
}

std::string extract_command(std::string message)
{
    std::istringstream iss(message);
    std::string comand;
    int count = 0;
    std::string result = "";

    while (iss >> comand) {
        count++;
        if (count > 3) {
            result += comand + " ";
        }
    }

    if (!result.empty()) {
        result.resize(result.length() - 1);
    }

    return result;
}

void handleMessage(const std::string& message)
{
	if (message.find("Se ha creado el canal ") != std::string::npos)
	{
		std::string canal = cleanMessage(message.substr(message.find("Se ha creado el canal ") + 22));

		bool canalExistente = false;
		for (size_t i = 0; i < known_channels.size(); ++i)
		{
			if (known_channels[i] == canal)
			{
				canalExistente = true;
				break;
			}
		}
		if (!canalExistente)
		{
			sendMessage("/JOIN " + canal);
			std::cout << bot_nick + " uniéndose automáticamente al nuevo canal detectado: " << canal << std::endl;
			known_channels.push_back(canal);
		}
		else
		{
			std::cout << "El canal '" << canal << "' ya está en known_channels de " + bot_nick +", no se envía JOIN." << std::endl;
		}
	}
	
	if (message.find("Te has unido al canal") != std::string::npos)
	{
		size_t pos = message.find("canal ");
		if (pos != std::string::npos)
		{
			std::string canal = cleanMessage(message.substr(pos + 6));
			sleep(1);
			sendMessage("/PRIVMSG #" + canal + " ¡Hola a todos! Soy " + bot_nick);
			sendMessage("/PRIVMSG #" + canal + " Comandos disponibles: !hora, !dado, !decide <opciones>");
			std::cout << bot_nick << " dio la bienvenida en el canal: " << canal << std::endl;
		}
	}
	
	std::string targetChannel = "";
	std::string command = extract_command(message);
	std::cout << command << ": " << command.length() << std::endl;
	
	if (message.find("se ha unido al canal") != std::string::npos)
	{
		size_t user_start = message.find("Usuario ") + 8;
		size_t user_end = message.find("se ha unido al canal");
		std::string user = cleanMessage(message.substr(user_start, user_end - user_start));

		std::string canal = cleanMessage(message.substr(message.find("al canal ") + 9));

		if (!canal.empty())
		{
			sleep(1);
			sendMessage("/PRIVMSG #" + canal + "¡¡" + user + " se ha unido al canal!!");
			sendMessage("/PRIVMSG #" + canal + " ¡Bienvenido, " + user + "! Soy " + bot_nick);
			sendMessage("/PRIVMSG #" + canal + " Comandos disponibles: !hora, !dado, !decide <opciones>");
		}
		return;
	}

	if (message.find("ha abandonado el canal") != std::string::npos)
	{
		size_t pos = message.find(' ');
		std::string user = cleanMessage(message.substr(0, pos));
		std::string canal = cleanMessage(message.substr(message.find("abandonado el canal ") + 19));
		sendMessage("/PRIVMSG #" + canal + " ¡" + user + " ha abandonado el canal! Sayonara, baby... 🤖");
		return;
	}

	size_t privmsgPos = message.find("PRIVMSG ");
	if (privmsgPos != std::string::npos)
	{
		std::string	private_target = "";
		size_t		senderStart = privmsgPos + 8;
		size_t		senderEnd = message.find('!', senderStart);


		if (senderEnd != std::string::npos)
		{
			private_target = cleanMessage(message.substr(senderStart, senderEnd - senderStart));
		}
	
		if (command == "!hora")
		{
			sendMessage("/PRIVMSG " + private_target + " La hora actual es " + getCurrentTime());
		}
		else if (command == "!dado")
		{
			int roll = rand() % 6 + 1;
			std::stringstream ss;
			ss << "/PRIVMSG " << private_target << " Has sacado un " << roll;
			sendMessage(ss.str());
		}
		else if (message.find("!decide") != std::string::npos)
		{
			if (std::isspace(command[7]))
			{
				size_t decide_start = message.find("!decide") + 8;
				std::vector<std::string> parts = split(message.substr(decide_start), ' ');
				if (parts.size() > 2)
				{
					int choice = rand() % (parts.size() - 1) + 1;
					sendMessage("/PRIVMSG " + private_target + " Decisión: " + parts[choice]);
				}
				else
					sendMessage("/PRIVMSG " + private_target + " Uso: !decide opción1 opción2 ...");
			}
		}
	}

	size_t chanmsgPos = message.find("#");
	if (chanmsgPos != std::string::npos)
	{
		std::string	channel_target = "";
		size_t		end = message.find(" ->");
		if (end != std::string::npos)
		{
			channel_target = message.substr(1, end);
		}
	
		if (command == "!hora")
		{
			sendMessage("/PRIVMSG " + channel_target + " La hora actual es " + getCurrentTime());
		}
		else if (command == "!dado")
		{
			int roll = rand() % 6 + 1;
			std::stringstream ss;
			ss << "/PRIVMSG " << channel_target << " Has sacado un " << roll;
			sendMessage(ss.str());
		}
		else if (message.find("!decide") != std::string::npos)
		{
			if (std::isspace(command[7]))
			{
				size_t decide_start = message.find("!decide") + 8;
				std::string options_str = cleanMessage(message.substr(decide_start));
				std::vector<std::string> parts = split(options_str, ' ');
				if (parts.size() >= 2 )
				{
					int choice = rand() % parts.size();
					std::string decision = parts[choice];
					sendMessage("/PRIVMSG " + channel_target + " Decisión: " + decision);
				}
				else
					sendMessage("/PRIVMSG " + channel_target + " Uso: !decide <opción1> <opción2> ...");
			}
		}
		else if (message.find("!usuarios") != std::string::npos)
		{
			sendMessage("/NAMES " + channel_target);
		}
	}
}

void* receiveLoop(void*)
{
    char buffer[1024];
    int bytes_received;

    while (running && (bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        handleMessage(message);
		std::cout << "📥 Mensaje recibido del servidor: " << message << std::endl;
    }

    if (bytes_received <= 0 && running)
    {
        std::cerr << "Desconectado del servidor.\n";
        running = false;
    }

    return NULL;
}

void launchBot(int port, std::string pass)
{
	srand(time(NULL));

	signal(SIGINT, signalHandler);

	std::string server_ip = "127.0.0.1";

	struct sockaddr_in server_addr;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		std::cerr << "Error al crear el socket\n";
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

	int	attempts = 5;
	while (attempts--)
	{
		if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0)
		{
			std::cout << "Bot conectado correctamente al servidor.\n";
			break;
		}
		std::cerr << "\nError al conectar con el servidor, reintentando...\n";
		sleep(1);
	}
	if (attempts <= 0)
	{
		std::cerr << "\nNo se pudo conectar al servidor después de varios intentos.\n";
		close(sock);
		return;
	}
	
	std::cout << "Conectado al servidor IRC\n";

	sendMessage("/PASS " + pass);
	sendMessage("/NICK " + bot_nick);
	sendMessage("/USER bot");

	pthread_t recv_thread;
	pthread_create(&recv_thread, NULL, receiveLoop, NULL);

	pthread_join(recv_thread, NULL);

	std::cout << "[Bot] Cerrando bot limpio." << std::endl;
	close(sock);
}
