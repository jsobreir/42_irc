#ifndef SERVER_HPP
#define SERVER_HPP

#include "IRC.hpp"


class Client;

class Server {
	private:
		std::vector<Client *> _clients;
	public:
		Server ();
		Server (Server const &other);
		Server &operator=(Server const &other);
		~Server();
		void start();
		void handleClientMessage(int client_fd, const char *msg);
		Client *getClient(int fd);
};

#endif
