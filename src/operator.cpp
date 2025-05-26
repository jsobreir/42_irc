#include "IRC.hpp"

int Server::handleModeOperatorCMD(IRCCommand cmd, Client *client) {
	
	if (!cmd.args.size()) {
		// TODO - Send error: no channel specified
        #if DEBUG
			std::cout << "Entered No channel name if" << std::endl;
        #endif
		return 0;
	}
	std::string channelName = cmd.args[0];

	Channel* channel = getChannel(channelName);
	if (!channel) {
        #if DEBUG
            std::cout << "Entered No channel name if" << std::endl;
        #endif
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
		return 0;
	}
	if (cmd.args.size() < 2 || (cmd.args[1][0] != '+' && cmd.args[1][0] != '-') || cmd.args[1].size() < 2) {
		// TODO - Invalid mode format
        #if DEBUG
            std::cout << "[DBG] Invalid mode format" << std::endl;
        #endif
		return 0;
	}

	char mode = cmd.args[1][1];
	bool isAdding = (cmd.args[1][0] == '+');
	// Check if the sender is an operator in the channel
	if (!channel->isOperator(client)) {
		#if DEBUG
		std::cout << "[DBG] Is operator" << std::endl;
        #endif
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(channel->getName()));
		return 0;
	}

	if (mode == 'o') {
		if (cmd.args.size() < 3) {
			std::string msg = "Please enter an user.\r\n";
			send(client->getFd(), msg.c_str(), msg.size(), 0);
			return 0;
		}
		std::string targetNick = cmd.args[2];
        #if DEBUG
            std::cout << "[DBG] Mode o" << std::endl;
        #endif

		Client* targetClient = NULL;
		for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if ((*it)->getNick() == targetNick) {
				targetClient = *it;
				break;
			}
		}
		if (!targetClient) {
			// TODO - Send error: no such nick
			return 0;
		}

		if (isAdding) {
			channel->addOperator(targetClient);
		} else {
			channel->removeOperator(targetClient);
		}

		// Broadcast mode change
		std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + cmd.args[1] + " " + targetNick + "\r\n";
		for (size_t i = 0; i < channel->getClients().size(); ++i) {
			send(channel->getClients()[i]->getFd(), msg.c_str(), msg.length(), 0);
		}
	} else if (mode == 'i') {
		if (isAdding) {
			channel->setInviteOnly(true);
		} else {
			channel->setInviteOnly(false);
		}

		// Broadcast invite-only mode change (no targetNick needed)
		std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + cmd.args[1] + "\r\n";
		for (size_t i = 0; i < channel->getClients().size(); ++i) {
			send(channel->getClients()[i]->getFd(), msg.c_str(), msg.length(), 0);
		}
	} else if (mode == 'k') {
		if (!channel->isOperator(client)) {
			std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return 0;
		}
		if (cmd.args.size() < 3) {
			std::cout << "Bad Usage" << std::endl;
			return 1;
		}
		std::string password = cmd.args[2];
		
		if (isAdding) {
			if (password.empty()) {
				std::string err = ":server 696 " + client->getNick() + " " + channelName + " +k :Key (password) required\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return 0;
			}
			channel->setKey(password);
		} else {
			channel->setKey(""); // clear password
		}

		// Broadcast the change
		std::string reply = ":" + client->getNick() + " MODE " + channelName + " " + mode;
		if (!password.empty())
			reply += " " + password;
		reply += "\r\n";

		for (size_t i = 0; i < channel->getClients().size(); ++i) {
			send(channel->getClients()[i]->getFd(), reply.c_str(), reply.length(), 0);
		}
	}else if (mode == 'l') {
		if (isAdding) {
			// Ensure parameter for +l (user limit) is provided
			if (cmd.args.size() < 3) {
				std::string err = ":server 461 " + client->getNick() + " MODE :Not enough parameters for +l\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return 0;
			}
	
			std::string limitStr = cmd.args[2];
			for (size_t i = 0; i < limitStr.size(); ++i) {
				if (!isdigit(limitStr[i])) {
					std::string err = ":server 696 " + client->getNick() + " " + channelName + " +l :Invalid limit value\r\n";
					send(client->getFd(), err.c_str(), err.length(), 0);
					return 0;
				}
			}
	
			int limit = std::atoi(limitStr.c_str());
			if (limit <= 0) {
				std::string err = ":server 696 " + client->getNick() + " " + channelName + " +l :Limit must be greater than 0\r\n";
				send(client->getFd(), err.c_str(), err.length(), 0);
				return 0;
			}
	
			channel->setUserLimit(limit);
	
			// Broadcast limit set
			std::string msg = ":" + client->getNick() + " MODE " + channelName + " +l " + limitStr + "\r\n";
			const std::vector<Client*>& clients = channel->getClients();
			for (size_t i = 0; i < clients.size(); ++i)
				send(clients[i]->getFd(), msg.c_str(), msg.length(), 0);
		} else {
			// Remove the limit
			channel->setUserLimit(0);
	
			std::string msg = ":" + client->getNick() + " MODE " + channelName + " -l\r\n";
			const std::vector<Client*>& clients = channel->getClients();
			for (size_t i = 0; i < clients.size(); ++i)
				send(clients[i]->getFd(), msg.c_str(), msg.length(), 0);
		}
	}
	else {
		sendCMD(client->getFd(), ERR_UNKNOWNMODE(cmd.args[1]));
		return 0;
	}
	return 0;
}

int		Server::handleKickOperatorCMD(IRCCommand cmd, Client *client) {
	// USAGE: /KICK  <nickname> [:reason...]
	if (cmd.args.size() < 2) {
		std::string err = ":server 461 " + client->getNick() + " KICK :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}
	std::string channelName = cmd.args[0];
	std::string targetNick = cmd.args[1];

	// Check required params

	Channel* channel = getChannel(channelName);
	if (!channel) {
		std::string err = ":server 403 " + client->getNick() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	if (!channel->isOperator(client)) {
		std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
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
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	std::string reason;
	// Parse optional reason (rest of line)
	if (cmd.args.size() < 2) {
		if (!reason.empty() && reason[0] == ' ')
			reason = reason.substr(1);
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
	}

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
	return 0;
}

int		Server::handleTopicOperatorCMD(IRCCommand cmd, Client *client) {
	std::string channelName = cmd.args[0];

	if (channelName.empty()) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(cmd.args[0]));
		//continue;
	}

	Channel* channel = getChannel(channelName);
	if (!channel) {
		// Send error: No such channel (ERR_NOSUCHCHANNEL 403)
		std::string err = ":server 403 " + client->getNick() + " " + channelName + " :No such channel\r\n";
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
		//continue;
	}

	// Check if there is a new topic to set (after a colon)
	std::string restOfLine = cmd.args[1];
	if (!restOfLine.empty()) {
		for (int i = 1; (size_t)i - 1 < cmd.args[i].size(); i++) {
			
		}
		// Remove leading spaces and the colon if present
		size_t pos = restOfLine.find_first_not_of(" ");
		if (pos != std::string::npos)
			restOfLine = restOfLine.substr(pos);
		if (!restOfLine.empty() && restOfLine[0] == ':')
			restOfLine = restOfLine.substr(1);

		// Only allow topic change if the client is operator in the channel
		if (!channel->isOperator(client)) {
			sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(channelName));
			return 0;
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
			sendCMD(client->getFd(), RPL_NOTOPIC(channelName));
		} else {
			// RPL_TOPIC (332)
			std::string msg = ":server 332 " + client->getNick() + " " + channelName + " :" + topic + "\r\n";
			sendCMD(client->getFd(), RPL_TOPIC(channelName, cmd.args[1]));
		}
	}
	return 0;
}

int		Server::handleInviteOperatorCMD(IRCCommand cmd, Client *client) {
	(void)cmd;
	(void) client;
	return 1;
}

