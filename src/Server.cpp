#include "Server.hpp"
#include "Client.hpp"
#include "IRC.hpp"


#define MAX_CLIENTS 1024

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

Server::~Server() {}

int Server::getServerFd(void) const {
	return _server_fd;
}

void handleSIGINT(int sig) {
	std::cout << "Terminate." << std::endl;
	(void)sig;
	close(g_server->getServerFd());
	g_server->closeAllClientFds();
	exit(0);
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
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		close(_server_fd);
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
			std::string user_pass;
			ss >> user_pass; // get the password client sent (not the entire line)

			#if DEBUG
				std::cout << "[DBG]Client " << fd << " sent PASS: " << user_pass << std::endl;
			#endif

			if (user_pass == getPassword()) {
				client->setAuthenticated(true);
				client->setPasswd(user_pass);
				#if DEBUG
					std::cout << "[DBG]Client " << fd << " authenticated successfully." << std::endl;
				#endif
			} else {
				#if DEBUG
					std::cout << "[DBG]Client " << fd << " failed authentication with PASS." << std::endl;
				#endif
			}
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
				std::string welcome = RPL_WELCOME(client->getNick(), _serverName);
				send(fd, welcome.c_str(), welcome.length(), 0);

				// Host information
				std::string hostInfo = RPL_YOURHOST(_serverName);
				send(fd, hostInfo.c_str(), hostInfo.length(), 0);

				// Server creation date
				std::string creationDate = RPL_CREATED(_creationDate);
				send(fd, creationDate.c_str(), creationDate.length(), 0);

				// Server capabilities
				std::string capabilities = RPL_MYINFO(_serverName, client->getNick(), _serverVersion);
				send(fd, capabilities.c_str(), capabilities.length(), 0);

				// Message of the day (MOTD)
				std::string motdStart = RPL_MOTDSTART(client->getNick());
				send(fd, motdStart.c_str(), motdStart.length(), 0);

				std::string motd = RPL_MOTD(client->getNick());
				send(fd, motd.c_str(), motd.length(), 0);

				#if DEBUG
					std::cout << "[DBG]Sent full welcome sequence to client " << fd << std::endl;
				#endif
			}
		}

		else if (command == "JOIN") {
			std::string channelName;
			commandStream >> channelName;
		
			if (!channelName.empty()) {
				joinChannel(client, channelName);
				#if DEBUG
					std::cout << "[DBG]Client " << fd << " joined channel " << channelName << std::endl;
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
		else if (command == "PRIVMSG") {
			std::string target;
			commandStream >> target;

			std::string message;
			std::getline(commandStream, message); // message starts with space + ':'
			if (!message.empty() && message[0] == ' ')
				message.erase(0, 1);
			if (!message.empty() && message[0] == ':')
				message.erase(0, 1);

			if (!target.empty() && !message.empty()) {
				std::string prefix = ":" + client->getNick() + "!" + client->getUser() + "@localhost";
				std::string fullMsg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";

				// Handle only channels (targets starting with '#')
				if (target[0] == '#') {
					Channel *chan = getChannel(target); // You need this method to retrieve the channel
					if (chan) {
						const std::vector<Client *> &clients = chan->getClients();
						for (size_t i = 0; i < clients.size(); ++i) {
							if (clients[i]->getFd() != fd) { // don't echo back to sender
								send(clients[i]->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
							}
						}
					}
				}
			}
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

Channel* Server::getChannel(std::string channelName) {
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i].getName() == channelName) {
            return &_channels[i];
        }
    }
    return NULL;
}

// void Server::joinChannel(Client *client, const std::string &channelName) {
//     Channel* channel = getChannel(channelName);
//     if (!channel) {
//         Channel newChannel;              // create Channel object
//         newChannel.setName(channelName);
//         newChannel.addClient(client);
//         _channels.push_back(newChannel); // push copy into vector
//     } else {
//         channel->addClient(client);
//     }
// }

void Server::joinChannel(Client *client, const std::string &channelName) {
    Channel *channel = getChannel(channelName);
    if (!channel) {
        Channel newChannel;
        newChannel.setName(channelName);
        newChannel.addClient(client);
        _channels.push_back(newChannel);  // Store by value, not pointer
        channel = &_channels.back();
    } else {
        channel->addClient(client);
    }

    // Send the JOIN message itself — this tells the client it has joined the channel
    std::string prefix = ":" + client->getNick() + "!" + client->getUser() + "@localhost"; // Adjust host if you have it
    std::string joinMsg = prefix + " JOIN :" + channelName + "\r\n";
    send(client->getFd(), joinMsg.c_str(), joinMsg.length(), 0);

    // Optionally broadcast the JOIN to other clients already in the channel (excluding the joining client)
    const std::vector<Client*> &clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); i++) {
		Client *otherClient = clients[i];
		if (otherClient != client) {
			send(otherClient->getFd(), joinMsg.c_str(), joinMsg.length(), 0);
		}
	}
	

    // Send RPL_TOPIC (332) - For now no topic, send empty string
    std::string topic = ""; // You can extend Channel class to store a topic later
    std::string msg = ":server 332 " + client->getNick() + " " + channelName + " :" + topic + "\r\n";
    send(client->getFd(), msg.c_str(), msg.length(), 0);

    // Send RPL_NAMREPLY (353) - List of users
    std::string userList = "";
    for (size_t i = 0; i < clients.size(); i++) {
        if (i != 0) userList += " ";
        userList += clients[i]->getNick();
    }
    msg = ":server 353 " + client->getNick() + " = " + channelName + " :" + userList + "\r\n";
    send(client->getFd(), msg.c_str(), msg.length(), 0);

    // Send RPL_ENDOFNAMES (366)
    msg = ":server 366 " + client->getNick() + " " + channelName + " :End of /NAMES list\r\n";
    send(client->getFd(), msg.c_str(), msg.length(), 0);
}
