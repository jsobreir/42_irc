#include "IRC.hpp"

int Server::handleCapCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.size() < 1)
		return 0;
	if (cmd.args[1] == "LS") {
		std::string nickname = client->getNick();
		if (nickname.empty())
			nickname = "*"; // RFC fallback

		std::string capList = "multi-prefix sasl"; // IRCv3 capabilities
		std::string capResponse = "CAP " + nickname + " LS :" + capList + "\r\n";
		send(client->getFd(), capResponse.c_str(), capResponse.length(), 0);

		#if DEBUG
			std::cout << "[DBG]CAP LS response sent to client " << client->getFd() << ": " << capResponse << std::endl;
		#endif
	}
	return 0;
}

int    Server::handlePassCMD(IRCCommand cmd, Client *client) {
	#if DEBUG
		std::cout << "[DBG]Setting password for client " << client->getFd() << std::endl;
	#endif
	client->setPasswd(cmd.args[0]);
	return 0;
}

int Server::handleNickCMD(IRCCommand cmd, Client *client) {
	client->setNick(cmd.args[0]);
	#if DEBUG
		std::cout << "[DBG]Client " << client->getFd() << " set nickname to " << client->getNick() << std::endl;
	#endif
	return 0;
}

int Server::handleUserCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.size() < 1)
		return 0;
	client->setUser(cmd.args[0]);
	#if DEBUG
		std::cout << "[DBG]Client " << client->getFd() << " set username to " << cmd.args[0] << std::endl;
	#endif

	if (!client->getNick().empty() && !client->getUser().empty()) {
		// Welcome message
		std::string welcome = RPL_WELCOME(client->getNick(), _serverName);
		send(client->getFd(), welcome.c_str(), welcome.length(), 0);

		// Host information
		std::string hostInfo = RPL_YOURHOST(_serverName);
		send(client->getFd(), hostInfo.c_str(), hostInfo.length(), 0);

		// Server creation date
		std::string creationDate = RPL_CREATED(_creationDate);
		send(client->getFd(), creationDate.c_str(), creationDate.length(), 0);

		// Server capabilities
		std::string capabilities = RPL_MYINFO(_serverName, client->getNick(), _serverVersion);
		send(client->getFd(), capabilities.c_str(), capabilities.length(), 0);

		// Message of the day (MOTD)
		std::string motdStart = RPL_MOTDSTART(client->getNick());
		send(client->getFd(), motdStart.c_str(), motdStart.length(), 0);

		std::string motd = RPL_MOTD(client->getNick());
		send(client->getFd(), motd.c_str(), motd.length(), 0);

		#if DEBUG
			std::cout << "[DBG]Sent full welcome sequence to client " << client->getFd() << std::endl;
		#endif
	}
	return 0;
}

int Server::handleJoinCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.size() < 1) {
		// ERR_NEEDMOREPARAMS (461): Not enough parameters
		send(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command).c_str(), ERR_NEEDMOREPARAMS(cmd.command).length(), 0);
		return 0;
	}

	std::stringstream channelStream(cmd.args[0]);
	std::string channelName, key;


	while (std::getline(channelStream, channelName, ',')) {
		// Trim leading and trailing whitespace
		size_t start = channelName.find_first_not_of(" ");
		size_t end = channelName.find_last_not_of(" ");
		if (start == std::string::npos || end == std::string::npos) {
			sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
			continue;
		}
		channelName = channelName.substr(start, end - start + 1);
		if (cmd.args.size() == 2)
			key = cmd.args[1];
		// Check if the client is banned from the channel
		Channel* channel = getChannel(channelName);
		if (channel && !channel->getKey().empty()) {
			std::cout << "Key =" << key << std::endl;
			std::cout << "channelKey = "<< channel->getKey() << std::endl;
			if (key != channel->getKey()) {
				std::string err = ":server 475 " + client->getNick() + " " + channelName + " :Cannot join channel (+k) - incorrect key\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				continue;
			}
		}
		// Validate channel name (e.g., must start with '#')
		if (channelName[0] != '#') {
			sendCMD(client->getFd(), ERR_BADCHANMASK(channelName));
			continue;
		}

		std::string key;
		if (cmd.args.size() > 2) {
			std::stringstream buf(cmd.args[2]);
			// Check if a key is provided for this channel
			if (std::getline(buf, key, ',')) {
				// Validate the key if necessary (e.g., compare with stored channel key)
				// TODO: Add key validation logic if required
			}
		}

		// Check if the client is banned from the channel
		if (channel && channel->isBanned(client)) {
			sendCMD(client->getFd(), ERR_BANNEDFROMCHAN(channelName));
			continue;
		}

		// Check if the channel is full
		if (channel && channel->isFull()) {
			sendCMD(client->getFd(), ERR_CHANNELISFULL(channelName));
			continue;
		}

		// Check if the channel is invite-only
		if (channel && channel->isInviteOnly() && !channel->isInvited(client)) {
			sendCMD(client->getFd(), ERR_INVITEONLYCHAN(channelName));
			continue;
		}

		// Check if the client has exceeded the channel limit
		if (client->getChannelCount() >= MAX_CHANNELS) {
			sendCMD(client->getFd(), ERR_TOOMANYCHANNELS(channelName));
			continue;
		}

		// Join the channel
		joinChannel(client, channelName);
		#if DEBUG
			std::cout << "[DBG]Client " << client->getFd() << " joined channel " << channelName << std::endl;
		#endif
	}
	return 0;
}

int Server::handleQuitCMD(IRCCommand cmd, Client *client) {
	(void)cmd;
	#if DEBUG
		std::cout << "[DBG]Client " << client->getFd() << " disconnected" << std::endl;
	#endif
	close(client->getFd());
	return 0;
}

int Server::handlePrivMsgCMD(IRCCommand cmd, Client *client) {
	std::string target = cmd.args[0];
	std::string message = cmd.args[1];

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
			if (channelClients[i]->getFd() != client->getFd()) {
				send(channelClients[i]->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
			}
		}
	}
	return 0;
}