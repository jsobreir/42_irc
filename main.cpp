#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <cstdlib>

#include "IRC.hpp"

Server *g_server = NULL;
std::string _creationDate_g;
std::string _serverName_g;

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int port = std::atoi(argv[1]);
	if (port <= 0 || port > 65535) {
		std::cerr << "Invalid port number: must be between 1 and 65535." << std::endl;
		return 1;
	}

	std::string password(argv[2]);
	try
	{
		Server server(port, password);
		g_server = &server;
		_creationDate_g = g_server->getCreationDate();
		_serverName_g = g_server->getServerName();
		server.start();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	return 0;
}
