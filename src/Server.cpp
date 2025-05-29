#include "Server.hpp"
#include "Client.hpp"
#include "IRC.hpp"
#include "Messages.hpp"

#define MAX_CLIENTS 1024

std::string _serverName_g;
std::string _creationDate_g;

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
	_serverName_g = _serverName;
}

const std::string &Server::getPassword() const {
	return _password;
}


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
    for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++) {
        delete *it;
    }
	_channels.clear();
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		delete *it;
	}
	_clients.clear();
}

int Server::getServerFd(void) const {
	return _server_fd;
}

void handleSIGINT(int sig) {
	std::cout << "\nProgram terminated with CTL-C." << std::endl;
	(void)sig;
	close(g_server->getServerFd());
	g_server->closeAllClientFds();
	throw std::exception();

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
		signal(SIGINT, handleSIGINT);
		if (fds[0].revents & POLLIN)
			acceptNewClient(fds);
		// Handle client data
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

int Server::handleClientMessage(int fd, const char *msg) {
    std::stringstream ss(msg);
	std::string line;
    IRCCommand cmd;

    const int n_commands = 14;
    std::string commands[n_commands] = {"CAP", "PASS", "NICK", "USER", "JOIN", "QUIT", "PRIVMSG", "MODE", "TOPIC", "KICK", "INVITE", "PING", "PART"};
    Client *client = getClient(fd);

    while (std::getline(ss, line)) {
        cmd = parseIRCLine(line);

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
                break;
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
                std::cout << "[DBG]Unknown command: " << cmd.command << std::endl;
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

void Server::removeClient(Client *client) {
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if ((*it)->getFd() == client->getFd()) {
			Client *remove = *it;
			_clients.erase(it);
			delete remove;
			break;
		}
	}
}

Channel* Server::getChannel(std::string channelName) {
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i]->getName() == channelName) {
            return _channels[i];
        }
    }
    return NULL;
}

void Server::joinChannel(Client *client, const std::string &channelName) {

	// Check if the channel already exists
	Channel *channel = getChannel(channelName);
	if (!channel) {
		// Create a new channel
		Channel *newChannel = new Channel();
		newChannel->setName(channelName);
		newChannel->addClient(client);
		_channels.push_back(newChannel); // Store the pointer in the vector
		channel = newChannel;
	} else {
		// Add the client to the existing channel
		channel->addClient(client);
	}

	// Increment the client's joined channels count
	client->incrementJoinedChannels();

    // Send the JOIN message itself — this tells the client it has joined the channel
    std::string prefix = ":" + client->getNick() + "!" + client->getUser() + "@localhost"; // Adjust host if you have it
    std::string joinMsg = prefix + " JOIN :" + channelName + "\r\n";
    send(client->getFd(), joinMsg.c_str(), joinMsg.length(), 0);

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
            userList += "@";  // prefix operator with @
        userList += clients[i]->getNick();
    }
    sendCMD(client->getFd(), RPL_NAMREPLY(client->getNick(), channelName, userList));

    sendCMD(client->getFd(), RPL_ENDOFNAMES(channelName));
}

void	Server::broadcastMsg(Channel *channel, std::string msg, Client *client) {
    const std::vector<Client*> &clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); i++) {
        Client *otherClient = clients[i];
		std::cout << "Other client is: " << otherClient->getNick() << " and user " <<  otherClient->getUser() << std::endl;
		std::cout << "client is: " << client->getNick() << std::endl;
        if (otherClient != client) {
			std::cout << "entered" << std::endl;
            sendCMD(otherClient->getFd(), msg);
        }
    }
	return ;
}