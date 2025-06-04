#include "Server.hpp"
#include "Client.hpp"
#include "IRC.hpp"
#include "Messages.hpp"

#define MAX_CLIENTS 1024

std::string _serverName_g;
std::string _creationDate_g;

Server::Server() {
	_nfds = 1;

	std::time_t now = std::time(0);
	struct tm *ltm = std::localtime(&now);

	// Format the time as "YYYY-MM-DD HH:MM:SS"
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
	_creationDate = buffer;

	_serverName = "42_ft_IRC";
	_serverVersion = "1.0";
}

Server::Server(int port, std::string password)
	: _nfds(1),
	  _server_fd(-1),
	  _serverName("42_ft_IRC"),
	  _port(port),
	  _password(password)
{
	std::time_t now = std::time(NULL);
	struct tm *ltm = std::localtime(&now);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
	_creationDate = buffer;
}

const std::string &Server::getPassword() const {
	return _password;
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

Server::~Server() {
	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete *it;
	}
	std::vector<Channel*>().swap(_channels);

	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete *it;
	}
	std::vector<Client*>().swap(_clients);
}


int Server::getServerFd(void) const {
	return _server_fd;
}

std::string Server::getServerName() const {
	return _serverName;
}

std::string Server::getCreationDate() const {
	return _creationDate;
}

void Server::start() {
	struct sockaddr_in server_addr;
	int port = 6667;

	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		perror("socket");
		throw std::exception();
	}

	// Set SO_REUSEADDR to allow immediate reuse of the port
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		close(_server_fd);
		close(_server_fd);
		throw std::exception();
		return;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		close(_server_fd);
		throw std::exception();
	}

	if (listen(_server_fd, 5) < 0) {
		perror("listen");
		close(_server_fd);
		throw std::exception();
	}

	std::cout << "Server listening on port " << port << std::endl;

	struct pollfd fds[MAX_CLIENTS];
	fds[0].fd = _server_fd;
	fds[0].events = POLLIN;


	while (1) {
		int activity = poll(fds, _nfds, -1);
		if (activity < 0) {
			perror("poll");
			throw std::exception();
		}
		if (fds[0].revents & POLLIN)
			acceptNewClient(fds);
		handleClientData(fds);
	}
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
		std::string buffer;
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
				std::string appendString(buffer);
				getClient(fds[i].fd)->appendBuffer(appendString);
				handleClientMessage(fds[i].fd);
			}
		}
	}
}

int Server::handleClientMessage(int fd) {
	std::string line;
	IRCCommand cmd;

	const int n_commands = 14;
	std::string commands[n_commands] = {"CAP", "PASS", "NICK", "USER", "JOIN", "QUIT", "PRIVMSG", "MODE", "TOPIC", "KICK", "INVITE", "PING", "PART"};
	Client *client = getClient(fd);

	size_t pos = client->getBuffer().find('\n');
	while (pos != std::string::npos) {
		std::string line = client->getBuffer().substr(0, pos);
		client->getBuffer().erase(0, pos + 1);
		cmd = parseIRCLine(line);
		pos = client->getBuffer().find('\n');

		std::cout << cmd.command << std::endl;
		for (std::vector<std::string>::iterator it = cmd.args.begin(); it != cmd.args.end(); it++)
			std::cout << *it << std::endl;
		int i;
		for (i = 0; i < n_commands; i++) {
			if (cmd.command == commands[i])
				break;
		}
		switch (i)
		{
			case 0:
				handleCapCMD(cmd, client);
				break;
			case 1:
				handlePassCMD(cmd, client);
				break;
			case 2:
				handleNickCMD(cmd, client);
				break;
			case 3:
				handleUserCMD(cmd, client);
				break;
			case 4:
				handleJoinCMD(cmd, client);
				break;
			case 5:
				handleQuitCMD(cmd, client);
				//break;
				return 0;
			case 6:
				handlePrivMsgCMD(cmd, client);
				break;
			case 7:
				handleModeOperatorCMD(cmd, client);
				break;
			case 8:
				handleTopicOperatorCMD(cmd, client);
				break;
			case 9:
				handleKickOperatorCMD(cmd, client);
				break;
			case 10:
				handleInviteOperatorCMD(cmd, client);
				break;
			case 11:
				handlePingCMD(cmd, client);
				break;
			case 12:
				handlePartCMD(cmd, client);
				break;
			default:
				#if DEBUG
				std::cout << "[DBG - handleClientMessage]Unknown command: " << cmd.command << std::endl;
				#endif   
				break;
		}
	}
	return 0;
}

void Server::sendCMD(int fd, std::string msg) {
	size_t len = msg.length();
	send(fd, msg.c_str(), len, 0);
	return ;
}


void Server::closeAllClientFds() {
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (*it)
			close((*it)->getFd());
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

Client 	*Server::getClientByNick(const std::string &nickname) {
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if ((*it)->getNick() == nickname)
			return *it;
	}
	return NULL;
}

void Server::removeClient(Client *client) {
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if ((*it)->getFd() == client->getFd()) {
			delete *it;
			_clients.erase(it);
			break;
		}
	}
}

Channel* Server::getChannel(std::string channelName) {
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i]->getName() == channelName) {
			#if DEBUG
				std::cout << "[DBG - getChannel] Found channel: " << channelName << std::endl;
			#endif
			return _channels[i];
		}
	}
	#if DEBUG
		std::cerr << "[DBG - getChannel] Channel not found: " << channelName << std::endl;
	#endif
	return NULL;
}

void Server::joinChannel(Client *client, const std::string &channelName) {

	Channel *channel = getChannel(channelName);
	if (!channel) {
		Channel *newChannel = new Channel();
		newChannel->setName(channelName);
		newChannel->addClient(client);
		_channels.push_back(newChannel);
		channel = newChannel;
	} else {
		channel->addClient(client);
	}

	client->incrementJoinedChannels();

    std::string prefix = ":" + client->getNick() + "!" + client->getUser() + "@localhost";
    std::string joinMsg = prefix + " JOIN :" + channelName + "\r\n";
    sendCMD(client->getFd(), joinMsg);

	broadcastMsg(channel, joinMsg, client);

	std::string topic = channel->getTopic();
	sendCMD(client->getFd(), RPL_TOPIC2(client->getNick(), channelName, topic));

	const std::vector<Client*> &clients = channel->getClients();
	
	std::string userList = "";
    Client* op = channel->getOperator();
    for (size_t i = 0; i < clients.size(); i++) {
        if (i != 0)
            userList += " ";
        if (clients[i] == op)
            userList += "@";
        userList += clients[i]->getNick();
    }
    sendCMD(client->getFd(), RPL_NAMREPLY(client->getNick(), channelName, userList));

	sendCMD(client->getFd(), RPL_ENDOFNAMES(client->getNick(), channelName));
}

void Server::broadcastMsg(Channel *channel, std::string msg, Client *client) {
	const std::vector<Client*> &clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); i++) {
		Client *otherClient = clients[i];
		if (otherClient == NULL) {
			#if DEBUG
				std::cerr << "[DBG - broadcastMsg] Null client found in channel!" << std::endl;
			#endif
			continue;
		}

		#if DEBUG
			std::cout << "Other client is: " << otherClient->getNick() << " and user " << otherClient->getUser() << std::endl;
			std::cout << "client is: " << client->getNick() << std::endl;
		#endif

		if (otherClient != client) {
			#if DEBUG
				std::cout << "entered" << std::endl;
			#endif
			sendCMD(otherClient->getFd(), msg);
		}
	}
}
