#include "Server.hpp"
#include "Client.hpp"
#include "IRC.hpp"


#define MAX_CLIENTS 1024

Server::Server() {
	_nfds = 1;
}

Server::Server(Server const &other) {
	_nfds = other._nfds;
}

Server &Server::operator=(Server const &other) {
	if (this != &other) {
		_nfds = other._nfds;
		_clients = other._clients;
		_channels = other._channels;
	}
	return *this;
}

Server::~Server() {}

int Server::getServerFd(void) const {
	return _server_fd;
}
void Server::start() {
	struct sockaddr_in server_addr;
	//socklen_t client_addr_len = sizeof(sockaddr_in);
	int port = 6667;

	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		perror("socket");
		return;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		close(_server_fd);
		return;
	}

	if (listen(_server_fd, 5) < 0) {
		perror("listen");
		close(_server_fd);
		return;
	}

	std::cout << "Server listening on port " << port << std::endl;

	struct pollfd fds[MAX_CLIENTS];
	fds[0].fd = _server_fd;
	fds[0].events = POLLIN;


	while (1) {
		int activity = poll(fds, _nfds, -1);
		if (activity < 0) {
			perror("poll");
			break;
		}
		signal(SIGINT, handleSIGINT);
		if (fds[0].revents & POLLIN)
			acceptNewClient(fds);
		// Handle client data
		handleClientData(fds);
	}
}

void handleSIGINT(int sig) {
	std::cout << "Terminate." << std::endl;
	(void)sig;
	close(g_server->getServerFd());
	g_server->closeAllClientFds();
	exit(0);
}

void Server::acceptNewClient(struct pollfd fds[]) {
	int client_fd;
	client_fd = accept(_server_fd, NULL, NULL);
	if (_nfds < MAX_CLIENTS) {
		fds[_nfds].fd = client_fd;
		fds[_nfds].events = POLLIN;
		_nfds++;

		Client *new_client = new Client(client_fd);
		_clients.push_back(new_client);
	} else {
		close(client_fd);
	}
}

void Server::handleClientData(struct pollfd fds[]) {
	for (int i = 1; i < _nfds; i++) {
		if (fds[i].revents & POLLIN) {
			char buffer[1024];
			int bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
			if (bytes <= 0) {
				close(fds[i].fd);
				fds[i] = fds[_nfds - 1];
				_nfds--;
				i--;
			} else {
				std::cout << "Received message from client: " << buffer << std::endl;
				buffer[bytes] = '\0';
				handleClientMessage(fds[i].fd, buffer);
			}
		}
	}
}

int Server::handleClientMessage(int fd, const char *msg)
{
	std::stringstream ss(msg);
	std::string command;
	ss >> command;

	Client *client = getClient(fd);
	if (!client)
		return 0;
	if (command == "PASS") {
		std::cout << "[DBG]Setting password for client " << fd << std::endl;
		client->setPasswd(msg);
	}
	else if (command == "NICK") {
		std::string nickname;
		ss >> nickname;
		client->setNick(nickname);
		std::cout << "[DBG]Client " << fd << " set nickname to " << nickname << std::endl;
	}
	else if (command == "USER") {
		std::string username;
		ss >> username;
		client->setUser(username);
		std::cout << "[DBG]Client " << fd << " set username to " << username << std::endl;
		if (!client->getNick().empty() && !client->getUser().empty()) {
			std::string welcome = ":localhost 001 " + client->getNick() + " :Welcome to the IRC server\r\n";
			send(fd, welcome.c_str(), welcome.length(), 0);
		}
	}
	else if (command == "JOIN") {
		std::string channel;
		ss >> channel;
		std::cout << "[DBG]Client " << fd << " joined channel " << channel << std::endl;
		std::string reply = RPL_JOIN(client->getNick(), client->getUser(), "localhost", channel);
		send(fd, reply.c_str(), reply.length(), 0);
	}
	else if (command == "DEBUG") {
		for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			std::cout << "Client fd: " << (*it)->getFd() << std::endl;
		}
	}
	else if (command == "PRIVMSG") {
		std::string target, msg_body;
		ss >> target;
		std::getline(ss, msg_body);
		
		// Remove leading colon and spaces
		if (!msg_body.empty() && msg_body[0] == ':')
			msg_body = msg_body.substr(1);
		
		// Trim leading spaces (optional)
		while (!msg_body.empty() && isspace(msg_body[0]))
			msg_body.erase(0, 1);
	
		if (target.empty() || msg_body.empty()) {
			std::string error = "461 " + client->getNick() + " PRIVMSG :Not enough parameters\r\n";
			send(fd, error.c_str(), error.length(), 0);
			return 0;
		}
		// Sending to a channel
		if (target[0] == '#') {
			// Broadcast to all users in channel except sender
			for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
				if ((*it)->getFd() != fd) {
					std::string message = ":" + client->getNick() + "!" + client->getUser() + "@localhost PRIVMSG " + target + " :" + msg_body + "\r\n";
					send((*it)->getFd(), message.c_str(), message.length(), 0);
				}
			}
		} else {
			// Direct message to user
			for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
				if ((*it)->getNick() == target) {
					std::string message = ":" + client->getNick() + "!" + client->getUser() + "@localhost PRIVMSG " + target + " :" + msg_body + "\r\n";
					send((*it)->getFd(), message.c_str(), message.length(), 0);
					return 0;
				}
			}
			std::string error = "401 " + client->getNick() + " " + target + " :No such nick/channel\r\n";
			send(fd, error.c_str(), error.length(), 0);
		}
	}
	else {
		std::cout << "[DBG]Unknown command: " << command << std::endl;
	}
	return 0;
}

void Server::closeAllClientFds() {
	for (int i = 0; i < _nfds; i++) {
		if (_clients[i])
			close(_clients[i]->getFd());
	}
	return ;
}

Client *Server::getClient(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++) {
		if (_clients[i]->getFd() == fd) {
			return _clients[i];
		}
	}
	return NULL;
}
Channel *Server::getChannel(std::string channelName)
{
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i]->getName() == channelName) {
			return _channels[i];
		}
	}
	return NULL;
}

void Server::joinChannel(Client *client, const std::string &channelName) {
	if (!getChannel(channelName)) {
		Channel newChannel;
		newChannel.setName(channelName);
		newChannel.addClient(client);
		_channels.push_back(&newChannel);
	}
	else {
		getChannel(channelName)->addClient(client);
	}
}

