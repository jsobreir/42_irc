#include "IRC.hpp"

int Server::handleModeOperatorCMD(IRCCommand cmd, Client *client) {
	if (!cmd.args.size()) {
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
		#if DEBUG
			std::cout << "[DBG] Invalid mode format" << std::endl;
		#endif
		return 0;
	}

	char mode = cmd.args[1][1];
	bool isAdding = (cmd.args[1][0] == '+');

	if (!channel->isOperator(client)) {
		#if DEBUG
			std::cout << "[DBG] Is operator" << std::endl;
		#endif
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(client->getNick() , channel->getName()));
		return 0;
	}

	if (mode == 'i') {
		if (isAdding)
			channel->setInviteOnly(true);
		else
			channel->setInviteOnly(false);

		std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + cmd.args[1] + "\r\n";
		for (size_t i = 0; i < channel->getClients().size(); ++i) {
			send(channel->getClients()[i]->getFd(), msg.c_str(), msg.length(), 0);
		}
	}
	else if (mode == 't') {
		if (isAdding)
			channel->setTopicOnlyOps(true);
		else
			channel->setTopicOnlyOps(false);

		std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + cmd.args[1] + "\r\n";
		const std::vector<Client*>& clients = channel->getClients();
		for (size_t i = 0; i < clients.size(); ++i) {
			send(clients[i]->getFd(), msg.c_str(), msg.length(), 0);
		}
	}
	else if (mode == 'k') {
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

		std::string reply = ":" + client->getNick() + " MODE " + channelName + " " + mode;
		if (!password.empty())
			reply += " " + password;
		reply += "\r\n";

		for (size_t i = 0; i < channel->getClients().size(); ++i) {
			send(channel->getClients()[i]->getFd(), reply.c_str(), reply.length(), 0);
		}
	}
	else if (mode == 'o') {
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
			std::string err = ERR_NOSUCHNICK(client->getNick(), targetNick);
			send(client->getFd(), err.c_str(), err.length(), 0);
			return 0;
		}

		if (isAdding)
			channel->addOperator(targetClient);
		else
			channel->removeOperator(targetClient);

		std::string msg = ":" + client->getNick() + " MODE " + channelName + " " + cmd.args[1] + " " + targetNick + "\r\n";
		for (size_t i = 0; i < channel->getClients().size(); ++i) {
			send(channel->getClients()[i]->getFd(), msg.c_str(), msg.length(), 0);
		}
	}
	else if (mode == 'l') {
		if (isAdding) {
			if (cmd.args.size() < 3) {
				std::string err = ERR_NEEDMOREPARAMS("MODE");
				send(client->getFd(), err.c_str(), err.length(), 0);
				return 0;
			}

			std::string limitStr = cmd.args[2];
			for (size_t i = 0; i < limitStr.size(); ++i) {
				if (!isdigit(limitStr[i])) {
					//std::string err = ":server 696 " + client->getNick() + " " + channelName + " +l :Invalid limit value\r\n";
					std::string err = ERR_INVALIDMODEPARAM(client->getNick(), channelName, "+l", "value", "Invalid limit value");
					send(client->getFd(), err.c_str(), err.length(), 0);
					return 0;
				}
			}

			int limit = std::atoi(limitStr.c_str());
			if (limit <= 0) {
				//std::string err = ":server 696 " + client->getNick() + " " + channelName + " +l :Limit must be greater than 0\r\n";
				std::string err = ERR_INVALIDMODEPARAM(client->getNick(), channelName, "+l", "value", "Invalid limit value");
				send(client->getFd(), err.c_str(), err.length(), 0);
				return 0;
			}

			channel->setUserLimit(limit);
			std::string msg = ":" + client->getNick() + " MODE " + channelName + " +l " + limitStr + "\r\n";
			const std::vector<Client*>& clients = channel->getClients();
			for (size_t i = 0; i < clients.size(); ++i)
				send(clients[i]->getFd(), msg.c_str(), msg.length(), 0);
		} else {
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

int Server::handleKickOperatorCMD(IRCCommand cmd, Client *client) {
	// USAGE: /KICK <channel> <nickname> [:reason...]
	if (cmd.args.size() < 2) {
		send(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command).c_str(), ERR_NEEDMOREPARAMS(cmd.command).length(), 0);
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
		std::string err = ERR_CHANOPRIVSNEEDED(client->getNick(), channelName);
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
		std::string err = ERR_USERNOTINCHANNEL(client->getNick(), targetNick, channelName);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Parse optional reason
	std::string reason;
	if (cmd.args.size() > 2) {
		reason = cmd.args[2];
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

int Server::handleTopicOperatorCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.empty()) {
		// Not enough parameters: need at least the channel name
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS("TOPIC"));
		return 0;
	}

	std::string channelName = cmd.args[0];
	Channel* channel = getChannel(channelName);
	if (!channel) {
		// No such channel
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
		return 0;
	}

	// If only one argument: user wants to get the topic
	if (cmd.args.size() == 1) {
		std::string topic = channel->getTopic();
		if (topic.empty()) {
			sendCMD(client->getFd(), RPL_NOTOPIC(client->getNick(), channelName));
		} else {
			sendCMD(client->getFd(), RPL_TOPIC(client->getNick(), channelName, topic));
		}
		return 0;
	}

	if (channel->isTopicOnlyOps() && !channel->isOperator(client)) {
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(client->getNick(), channelName));
		return 0;
	}

	// Reconstruct topic from args[1..n]
	std::string newTopic;
	for (size_t i = 1; i < cmd.args.size(); ++i) {
		if (i > 1)
			newTopic += " ";
		newTopic += cmd.args[i];
	}

	// If it starts with a colon, remove it
	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic = newTopic.substr(1);

	// Set the new topic
	channel->setTopic(newTopic);

	// Broadcast topic change
	std::string topicMsg = ":" + client->getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	const std::vector<Client*>& clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); ++i) {
		send(clients[i]->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
	}

	return 0;
}

int Server::handleInviteOperatorCMD(IRCCommand cmd, Client *client) {
	// Ensure the command has the required arguments: target nickname and channel name
	if (cmd.args.size() < 2) {
		std::string err = ERR_NEEDMOREPARAMS("INVITE");
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	std::string targetNick = cmd.args[0];
	std::string channelName = cmd.args[1];

	// Check if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel) {
		std::string err = ERR_NOSUCHCHANNEL(channelName);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Check if the client is an operator in the channel
	if (!channel->isOperator(client)) {
		std::string err = ERR_CHANOPRIVSNEEDED(client->getNick(), channelName);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Find the target client by nickname
	Client* targetClient = NULL;
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getNick() == targetNick) {
			targetClient = *it;
			break;
		}
	}

	if (!targetClient) {
		std::string err = ERR_NOSUCHNICK(client->getNick(), targetNick);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Check if the target client is already in the channel
	if (channel->hasClient(targetClient)) {
		std::string err = ERR_USERONCHANNEL(targetNick, channelName);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Add the target client to the channel's invite list
	channel->inviteClient(targetNick);

	// Notify the target client about the invitation
	std::string inviteMsg = NTFY_CLIENTISINVITED(client->getNick(), targetNick, channel->getName());
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);

	// Notify the inviter that the invitation was successful
	std::string testMessage = RPL_INVITING("42_ft_IRC", client->getNick(), targetNick, channel->getName());
	send(client->getFd(), testMessage.c_str(), testMessage.length(), 0);

	return 0;
}