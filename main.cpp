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

void Server::clear_data() {
	closeAllClientFds();
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++) {
		Channel *temp = *it;
		if (temp)
			delete temp;
	}
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		Client *temp = *it;
		if (temp)
			delete temp;
	}
}

void cleanup_and_exit(int signum) {
	g_server->clear_data();
	if (g_server) {
		delete g_server;
		g_server = NULL;
	}
	std::cout << "\nServer shutting down due to CTRL+C " << signum << std::endl;
	std::exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	std::signal(SIGINT, cleanup_and_exit);
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
	try {
		g_server = new Server(port, password);
		g_server->start();
		delete g_server;
		g_server = NULL;
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	return 0;
}
