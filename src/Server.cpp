#include "Server.hpp"
#include "Client.hpp"
#include "IRC.hpp"


#define MAX_CLIENTS 1024

Server::Server() {
	_nfds = 1;

	std::time_t now = std::time(0);
	struct tm *ltm = std::localtime(&now);

	// Format the time as "YYYY-MM-DD HH:MM:SS"
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
	_creationDate = buffer;

	_serverName = "42_ft_IRC";
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
	int port = 6667;

	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		perror("socket");
		return;
	}

	// Set SO_REUSEADDR to allow immediate reuse of the port
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		close(server_fd);
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
	std::string line;

	// Process each line (command) separately
	while (std::getline(ss, line)) {
		// Remove any trailing carriage return
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		std::stringstream commandStream(line);
		std::string command;
		commandStream >> command;

		Client *client = getClient(fd);
		if (!client)
			return 0;

		if (command == "CAP") {
			std::string subcommand;
			commandStream >> subcommand;
			if (subcommand == "LS") {
				std::string nickname = client->getNick();
				if (nickname.empty())
					nickname = "*"; // RFC fallback
		
				std::string capList = "multi-prefix sasl"; // IRCv3 capabilities
				std::string capResponse = "CAP " + nickname + " LS :" + capList + "\r\n";
				send(fd, capResponse.c_str(), capResponse.length(), 0);
		
				#if DEBUG
					std::cout << "[DBG]CAP LS response sent to client " << fd << ": " << capResponse << std::endl;
				#endif
			}
		}

		else if (command == "PASS") {
			#if DEBUG
				std::cout << "[DBG]Setting password for client " << fd << std::endl;
			#endif
			client->setPasswd(line);
		}

		else if (command == "NICK") {
			std::string nickname;
			commandStream >> nickname;
			client->setNick(nickname);
			#if DEBUG
		   		std::cout << "[DBG]Client " << fd << " set nickname to " << nickname << std::endl;
			#endif
		}

		else if (command == "USER") {
			std::string username;
			commandStream >> username;
			client->setUser(username);
			#if DEBUG
				std::cout << "[DBG]Client " << fd << " set username to " << username << std::endl;
			#endif

			if (!client->getNick().empty() && !client->getUser().empty()) {
				// Welcome message
				std::string welcome = ":localhost 001 " + client->getNick() + " :Welcome to " + _serverName + ", " + client->getNick() + "\r\n";
				send(fd, welcome.c_str(), welcome.length(), 0);

				// Host information
				std::string hostInfo = ":localhost 002 :Your host is " + _serverName + ", running version 1.0\r\n";
				send(fd, hostInfo.c_str(), hostInfo.length(), 0);

				// Server creation date
				std::string creationDate = ":localhost 003 :This server was created " + _creationDate + "\r\n";
				send(fd, creationDate.c_str(), creationDate.length(), 0);

				// Server capabilities
				std::string capabilities = ":localhost 004 " + client->getNick() + " 1.0 :Available user modes: io, channel modes: tkl\r\n";
				send(fd, capabilities.c_str(), capabilities.length(), 0);

				// Message of the day (MOTD)
				std::string motdStart = ":localhost 375 " + client->getNick() + " :- Server Message of the day -\r\n";
				send(fd, motdStart.c_str(), motdStart.length(), 0);

				std::string motd = ":localhost 372 " + client->getNick() + " : Welcome to " + _serverName + ", and remember what happens in " + _serverName + " stays in " + _serverName + " 😎.\r\n";
				send(fd, motd.c_str(), motd.length(), 0);

				#if DEBUG
					std::cout << "[DBG]Sent full welcome sequence to client " << fd << std::endl;
				#endif
			}
		}

		else if (command == "QUIT") {
			#if DEBUG
				std::cout << "[DBG]Client " << fd << " disconnected" << std::endl;
			#endif
			close(fd);
			return 0; // Prevent server shutdown
		}

		else {
			#if DEBUG
				std::cout << "[DBG]Unknown command: " << command << std::endl;
			#endif
		}
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

