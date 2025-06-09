#include "IRC.hpp"

Server *g_server = NULL;

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
