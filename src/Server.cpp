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

Server::~Server() {
    for (size_t i = 0; i < _channels.size(); ++i) {
        delete _channels[i];
    }
    _channels.clear();
}

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
	else if (command == "MODE") {
		std::string channelName;
		commandStream >> channelName;

		if (channelName.empty()) {
			// ERR_NEEDMOREPARAMS (461)
			std::string err = ":server 461 " + client->getNick() + " MODE :Not enough parameters\r\n";
			send(fd, err.c_str(), err.length(), 0);
			return 0;
		}

		Channel* channel = getChannel(channelName);
		if (!channel) {
			// ERR_NOSUCHCHANNEL (403)
			std::string err = ":server 403 " + client->getNick() + " " + channelName + " :No such channel\r\n";
			send(fd, err.c_str(), err.length(), 0);
			return 0;
		}

		std::string modeChange;
		commandStream >> modeChange;

		if (modeChange.empty() || (modeChange[0] != '+' && modeChange[0] != '-')) {
			// ERR_UNKNOWNMODE (472)
			std::string err = ":server 472 " + client->getNick() + " " + modeChange + " :Unknown MODE flag\r\n";
			send(fd, err.c_str(), err.length(), 0);
			return 0;
		}

		bool isAdding = (modeChange[0] == '+');
		char mode = modeChange[1];

		// Only channel operators can change modes
		if (!channel->isOperator(client)) {
			// ERR_CHANOPRIVSNEEDED (482)
			std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
			send(fd, err.c_str(), err.length(), 0);
			return 0;
		}

		if (mode == 'o') {
			std::string targetNick;
			commandStream >> targetNick;

			if (targetNick.empty()) {
				// ERR_NEEDMOREPARAMS (461)
				std::string err = ":server 461 " + client->getNick() + " MODE :Not enough parameters\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}

			Client* targetClient = NULL;
			for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
				if ((*it)->getNick() == targetNick) {
					targetClient = *it;
					break;
				}
			}
			if (!targetClient) {
				// ERR_NOSUCHNICK (401)
				std::string err = ":server 401 " + client->getNick() + " " + targetNick + " :No such nick/channel\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}

			if (isAdding)
				channel->addOperator(targetClient);
			else
				channel->removeOperator(targetClient);

			// Broadcast to all users in channel
			std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + modeChange + " " + targetNick + "\r\n";
			for (size_t i = 0; i < channel->getClients().size(); ++i) {
				send(channel->getClients()[i]->getFd(), msg.c_str(), msg.length(), 0);
			}

		} else if (mode == 'i') {
			channel->setInviteOnly(isAdding);

			std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + modeChange + "\r\n";
			for (size_t i = 0; i < channel->getClients().size(); ++i) {
				send(channel->getClients()[i]->getFd(), msg.c_str(), msg.length(), 0);
			}

		} else if (mode == 'b') {
			if (!channel->isOperator(client)) {
				std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}
			std::string mask;
			commandStream >> mask;
			if (isAdding) {
				if (mask.empty()) {
					// If no mask provided, list current bans
					const std::vector<std::string>& bans = channel->getBanMasks();
					for (size_t i = 0; i < bans.size(); ++i) {
						std::string line = ":server 367 " + client->getNick() + " " + channelName + " " + bans[i] + "\r\n";
						send(fd, line.c_str(), line.length(), 0);
					}
					std::string end = ":server 368 " + client->getNick() + " " + channelName + " :End of channel ban list\r\n";
					send(fd, end.c_str(), end.length(), 0);
				} else {
					channel->addBanMask(mask);
					std::string reply = ":server 324 " + client->getNick() + " " + channelName + " +b " + mask + "\r\n";
					send(fd, reply.c_str(), reply.length(), 0);
				}
			} else {
				if (mask.empty()) {
					// ERR_NEEDMOREPARAMS
					std::string err = ":server 461 " + client->getNick() + " MODE :Not enough parameters for -b\r\n";
					send(fd, err.c_str(), err.length(), 0);
					return 0;
				}
				channel->removeBanMask(mask);
				std::string reply = ":server 324 " + client->getNick() + " " + channelName + " -b " + mask + "\r\n";
				send(fd, reply.c_str(), reply.length(), 0);
			}
		} else if (mode == 'k') {
			if (!channel->isOperator(client)) {
				std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}

			std::string password;
			commandStream >> password;

			if (isAdding) {
				if (password.empty()) {
					std::string err = ":server 696 " + client->getNick() + " " + channelName + " +k :Key (password) required\r\n";
					send(fd, err.c_str(), err.length(), 0);
					return 0;
				}
				channel->setKey(password);
			} else {
				channel->setKey(""); // clear password
			}

			// Broadcast the change
			std::string reply = ":" + client->getNick() + " MODE " + channelName + " " + modeChange;
			if (!password.empty())
				reply += " " + password;
			reply += "\r\n";

			for (size_t i = 0; i < channel->getClients().size(); ++i) {
				send(channel->getClients()[i]->getFd(), reply.c_str(), reply.length(), 0);
			}
		}else {
			// ERR_UNKNOWNMODE (472)
			std::string err = ":server 472 " + client->getNick() + " " + std::string(1, mode) + " :is unknown mode char to me\r\n";
			send(fd, err.c_str(), err.length(), 0);
			return 0;
		}
}

		else if (command == "JOIN") {
			std::string channels, keys;
			commandStream >> channels >> keys;
		
			if (channels.empty()) {
				// ERR_NEEDMOREPARAMS (461): Not enough parameters
				std::string err = ":server 461 " + client->getNick() + " JOIN :Not enough parameters\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}
		
			std::stringstream channelStream(channels);
			std::stringstream keyStream(keys);
			std::string channelName, key;
		
			while (std::getline(channelStream, channelName, ',')) {
				// Trim leading and trailing whitespace
				size_t start = channelName.find_first_not_of(" ");
				size_t end = channelName.find_last_not_of(" ");
				if (start == std::string::npos || end == std::string::npos) {
					// ERR_NOSUCHCHANNEL (403): Invalid channel name
					std::string err = ":server 403 " + client->getNick() + " " + channelName + "No such channel\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
				channelName = channelName.substr(start, end - start + 1);
		
				// Validate channel name (e.g., must start with '#')
				if (channelName[0] != '#') {
					// ERR_BADCHANMASK (476): Invalid channel mask
					std::string err = ":server 476 " + client->getNick() + " " + channelName + "Invalid channel name\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
		
				// Check if a key is provided for this channel
				if (std::getline(keyStream, key, ',')) {
					// Validate the key if necessary (e.g., compare with stored channel key)
					// TODO: Add key validation logic if required
				}
				// Check if the client is banned from the channel
				Channel* channel = getChannel(channelName);
				if (channel && !channel->getKey().empty()) {
					if (key != channel->getKey()) {
						std::string err = ":server 475 " + client->getNick() + " " + channelName + " :Cannot join channel (+k) - incorrect key\r\n";
						send(fd, err.c_str(), err.length(), 0);
						continue;
					}
				}
				if (channel && channel->isBanned(client)) {
					// ERR_BANNEDFROMCHAN (474): Banned from channel
					std::string err = ":server 474 " + client->getNick() + " " + channelName + "You are banned from this channel\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
		
				// Check if the channel is full
				if (channel && channel->isFull()) {
					// ERR_CHANNELISFULL (471): Channel is full
					std::string err = ":server 471 " + client->getNick() + " " + channelName + "Channel is full\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
		
				// Check if the channel is invite-only
				if (channel && channel->isInviteOnly() && !channel->isInvited(client)) {
					// ERR_INVITEONLYCHAN (473): Invite-only channel
					std::string err = ":server 473 " + client->getNick() + " " + channelName + "Cannot join channel (invite-only)\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
		
				// Check if the client has exceeded the channel limit
				if (client->getChannelCount() >= MAX_CHANNELS) {
					// ERR_TOOMANYCHANNELS (405): Too many channels
					std::string err = ":server 405 " + client->getNick() + " " + channelName + "You have joined too many channels\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
		
				// Join the channel
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

	// Trim leading whitespace and colon
	if (!message.empty() && message[0] == ' ')
		message.erase(0, 1);
	if (!message.empty() && message[0] == ':')
		message.erase(0, 1);

	#if DEBUG
		std::cout << "[DBG] PRIVMSG - target: " << target << std::endl;
		std::cout << "[DBG] PRIVMSG - message: <" << message << ">" << std::endl;
	#endif

	if (!target.empty() && !message.empty()) {
		std::string prefix = ":" + client->getNick() + "!" + client->getUser() + "@localhost";
		std::string fullMsg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";

		// Handle only channels (targets starting with '#')
		if (target[0] == '#') {
			Channel *channel = getChannel(target);
			if (!channel) {
				// Send error: no such channel (ERR_NOSUCHCHANNEL 403)
				std::string err = ":server 403 " + client->getNick() + " " + target + " :No such channel\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}

			const std::vector<Client *> &clients = channel->getClients();
			for (size_t i = 0; i < clients.size(); ++i) {
				if (clients[i]->getFd() != fd) { // don't echo back to sender
					send(clients[i]->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
				}
			}
		} else {
			// Handle private message to a user
			Client* targetClient = NULL;
			for (size_t i = 0; i < _clients.size(); ++i) {
				if (_clients[i]->getNick() == target) {
					targetClient = _clients[i];
					break;
				}
			}

			if (!targetClient) {
				// Send error: no such nick (ERR_NOSUCHNICK 401)
				std::string err = ":server 401 " + client->getNick() + " " + target + " :No such nick\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}

			send(targetClient->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
		}
	}
}
			else if (command == "TOPIC") {
			std::string channelName;
			commandStream >> channelName;
		
			if (channelName.empty()) {
				// Send error: No channel specified (ERR_NEEDMOREPARAMS 461)
				std::string err = ":server 461 " + client->getNick() + " TOPIC :Not enough parameters\r\n";
				send(fd, err.c_str(), err.length(), 0);
				continue;
			}
		
			Channel* channel = getChannel(channelName);
			if (!channel) {
				// Send error: No such channel (ERR_NOSUCHCHANNEL 403)
				std::string err = ":server 403 " + client->getNick() + " " + channelName + " :No such channel\r\n";
				send(fd, err.c_str(), err.length(), 0);
				continue;
			}
		
			// Check if there is a new topic to set (after a colon)
			std::string restOfLine;
			std::getline(commandStream, restOfLine);
			if (!restOfLine.empty()) {
				// Remove leading spaces and the colon if present
				size_t pos = restOfLine.find_first_not_of(" ");
				if (pos != std::string::npos)
					restOfLine = restOfLine.substr(pos);
				if (!restOfLine.empty() && restOfLine[0] == ':')
					restOfLine = restOfLine.substr(1);
		
				// Only allow topic change if the client is operator in the channel
				if (!channel->isOperator(client)) {
					// Send error: you must be channel operator (ERR_CHANOPRIVSNEEDED 482)
					std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
					send(fd, err.c_str(), err.length(), 0);
					continue;
				}
		
				// Set the new topic
				channel->setTopic(restOfLine);
		
				// Broadcast the topic change to all clients in the channel
				std::string topicMsg = ":" + client->getNick() + " TOPIC " + channelName + " :" + restOfLine + "\r\n";
				const std::vector<Client*>& clients = channel->getClients();
				for (size_t i = 0; i < clients.size(); ++i) {
					send(clients[i]->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
				}
			}
			else {
				// No topic argument - reply with current topic or no topic message
				std::string topic = channel->getTopic();
				if (topic.empty()) {
					// RPL_NOTOPIC (331)
					std::string msg = ":server 331 " + client->getNick() + " " + channelName + " :No topic is set\r\n";
					send(fd, msg.c_str(), msg.length(), 0);
				} else {
					// RPL_TOPIC (332)
					std::string msg = ":server 332 " + client->getNick() + " " + channelName + " :" + topic + "\r\n";
					send(fd, msg.c_str(), msg.length(), 0);
				}
			}
		}

		else if (command == "KICK") {
			// USAGE: /KICK  <nickname> [:reason...]
			std::string channelName, targetNick;
			commandStream >> channelName >> targetNick;
		
			// Check required params
			if (channelName.empty() || targetNick.empty()) {
				std::string err = ":server 461 " + client->getNick() + " KICK :Not enough parameters\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}
		
			Channel* channel = getChannel(channelName);
			if (!channel) {
				std::string err = ":server 403 " + client->getNick() + " " + channelName + " :No such channel\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}
		
			if (!channel->isOperator(client)) {
				std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}
		
			// Locate target client from the channel
			Client* targetClient = NULL;
			const std::vector<Client*>& clients = channel->getClients();
			for (size_t i = 0; i < clients.size(); ++i) {
				if (clients[i]->getNick() == targetNick) {
					targetClient = clients[i];
					break;
				}
			}
		
			if (!targetClient || !channel->hasClient(targetClient)) {
				std::string err = ":server 441 " + client->getNick() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
				send(fd, err.c_str(), err.length(), 0);
				return 0;
			}
		
			// Parse optional reason (rest of line)
			std::string reason;
			std::getline(commandStream, reason);
			if (!reason.empty() && reason[0] == ' ')
				reason = reason.substr(1);
			if (!reason.empty() && reason[0] == ':')
				reason = reason.substr(1);
		
			// Format KICK message
			std::string kickMsg = ":" + client->getNick() + " KICK " + channelName + " " + targetNick;
			if (!reason.empty())
				kickMsg += " :" + reason;
			kickMsg += "\r\n";
		
			// Broadcast to channel
			for (size_t i = 0; i < clients.size(); ++i)
				send(clients[i]->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
		
			// Send to kicked client as well (optional redundancy)
			send(targetClient->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
		
			// Remove user from channel
			channel->removeClient(targetClient);
			targetClient->decrementJoinedChannels();
		}
		
		else if (command == "PRIVMSG") {
			std::string target;
			commandStream >> target;
		
			std::string message;
			std::getline(commandStream, message);
		
			// Trim leading whitespace from the message
			size_t pos = message.find_first_not_of(" ");
			if (pos != std::string::npos) {
				message = message.substr(pos);
			}
		
			#if DEBUG
				std::cout << "[DBG] PRIVMSG - target: " << target << std::endl;
				std::cout << "[DBG] PRIVMSG - message: <" << message << ">" << std::endl;
			#endif		
		
			if (!message.empty() && message[0] == ':') {
				#if DEBUG
					std::cout << "[DBG] PRIVMSG - Removing leading colon from message" << std::endl;
				#endif
				message = message.substr(1); // Remove leading colon
			}
		
			if (target[0] == '#') {
				Channel *channel = getChannel(target);
				if (!channel) {
					// TODO - Send error: no such channel
					return 0;
				}
		
				// Construct the full message
				std::string fullMsg = ":" + client->getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
		
				// Broadcast to all clients in the channel **except the sender**
				std::vector<Client *> channelClients = channel->getClients();
				for (size_t i = 0; i < channelClients.size(); ++i) {
					if (channelClients[i]->getFd() != fd) {
						send(channelClients[i]->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
					}
				}
			}
		}
		else if (command == "PING") {
			std::string pingParam;
			std::getline(commandStream, pingParam);

			// Clean up the parameter (remove leading space and colon)
			if (!pingParam.empty() && pingParam[0] == ' ')
				pingParam.erase(0, 1);
			if (!pingParam.empty() && pingParam[0] == ':')
				pingParam.erase(0, 1);

			std::string response = "PONG :" + pingParam + "\r\n";
			send(fd, response.c_str(), response.length(), 0);
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
        if (_channels[i]->getName() == channelName) { // Use `->` to access members of the pointer
            return _channels[i]; // Return the pointer directly
        }
    }
    return NULL; // Return NULL if the channel is not found
}

void Server::joinChannel(Client *client, const std::string &channelName) {

    // Check for leading or trailing whitespace
    if (channelName.empty() || channelName.find_first_of(" ") == 0 || channelName.find_last_of(" ") == channelName.size() - 1) {
        std::string err = ":server " + ERR_BADCHANMASK(channelName);
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }

    // Validate channel name (e.g., must start with '#' and have more than just '#')
    if (channelName[0] != '#' || channelName.size() == 1) {
        std::string err = ":server " + ERR_BADCHANMASK(channelName);
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }

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

    // Broadcast the JOIN to other clients already in the channel (excluding the joining client)
    const std::vector<Client*> &clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); i++) {
        Client *otherClient = clients[i];
        if (otherClient != client) {
            send(otherClient->getFd(), joinMsg.c_str(), joinMsg.length(), 0);
        }
    }

    // Send RPL_TOPIC (332) - For now no topic, send empty string
    std::string topic = channel->getTopic(); // You can extend Channel class to store a topic later
    std::string msg = ":server " + RPL_TOPIC2(client->getNick(), channelName, topic);
    send(client->getFd(), msg.c_str(), msg.length(), 0);

    // Send RPL_NAMREPLY (353) - List of users
    std::string userList = "";
    Client* op = channel->getOperator();  // get operator client pointer
    for (size_t i = 0; i < clients.size(); i++) {
        if (i != 0)
            userList += " ";
        if (clients[i] == op)
            userList += "@";  // prefix operator with @
        userList += clients[i]->getNick();
    }
    msg = ":server " + RPL_NAMREPLY(client->getNick(), channelName, userList);
    send(client->getFd(), msg.c_str(), msg.length(), 0);

    // Send RPL_ENDOFNAMES (366)
    msg = ":server " + RPL_ENDOFNAMES(channelName);
    send(client->getFd(), msg.c_str(), msg.length(), 0);

	if(1 == 2)
	{
		int* ptr = NULL;
		*ptr = 42; 
	}
}
