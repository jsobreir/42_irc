#include "Server.hpp"
#include "Client.hpp"
#include "IRC.hpp"


#define MAX_CLIENTS 1024

Server::Server() {}

Server::Server(Server const &other) {
	(void)other;
}

Server &Server::operator=(Server const &other) {
	(void)other;
	return *this;
}

Server::~Server() {}

void Server::start() {
	int server_fd, client_fd;
	struct sockaddr_in server_addr;
	//socklen_t client_addr_len = sizeof(sockaddr_in);
	int port = 6667;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		return;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		close(server_fd);
		return;
	}

	if (listen(server_fd, 5) < 0) {
		perror("listen");
		close(server_fd);
		return;
	}

	std::cout << "Server listening on port " << port << std::endl;

	struct pollfd fds[MAX_CLIENTS];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	int nfds = 1;

	while (1) {
		int activity = poll(fds, nfds, -1);
		if (activity < 0) {
			perror("poll");
			break;
		}

		// Accept new client
		if (fds[0].revents & POLLIN) {
			client_fd = accept(server_fd, NULL, NULL);
			if (nfds < MAX_CLIENTS) {
				fds[nfds].fd = client_fd;
				fds[nfds].events = POLLIN;
				nfds++;

				Client *new_client = new Client(client_fd);
				_clients.push_back(new_client);
			} else {
				close(client_fd);
			}
		}

		// Handle client data
		for (int i = 1; i < nfds; i++) {
			if (fds[i].revents & POLLIN) {
				char buffer[1024];
				int bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				if (bytes <= 0) {
					close(fds[i].fd);
					fds[i] = fds[nfds - 1];
					nfds--;
					i--;
				} else {
					std::cout << "Received message from client: " << buffer << std::endl;
					buffer[bytes] = '\0';
					handleClientMessage(fds[i].fd, buffer);
				}
			}
		}
	}
}

void Server::handleClientMessage(int client_fd, const char *msg)
{
	std::stringstream ss(msg);
	std::string command;
	ss >> command;

	if (command == "JOIN")
	{
		std::string channel;
		ss >> channel;
		std::cout << RPL_JOIN(client);
	}
	else if (command == "NICK")
	{
		std::string nickname;
		ss >> nickname;
		std::cout << "Client " << client_fd << " set nickname to " << nickname << std::endl;
	}
	else if (command == "USER")
	{
		std::string username;
		ss >> username;
		std::cout << "Client " << client_fd << " set username to " << username << std::endl;
	}
	if (command == "PASS")
	{
		for (size_t i = 0; i < _clients.size(); i++)
		{
			if (_clients[i]->getFd() == client_fd)
			{
				std::cout << "Setting password for client " << client_fd << std::endl;
				_clients[i]->setPasswd(msg);
			}
		}
	}
}


/*Client *Server::getClient(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++) {
		if (_clients[i]->getFd() == fd) {
			return _clients[i];
		}
	}
	return NULL;
}*/
