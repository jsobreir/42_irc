#include "IRC.hpp"

int Server::handleModeOperatorCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.empty()) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command));
		#if DEBUG
			std::cout << "Entered No channel name if" << std::endl;
		#endif
		return 0;
	}

	std::string channelName = cmd.args[0];
	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
		return 0;
	}

	// Exception: If MODE has no params, display active channel modes
	if (cmd.args.size() == 1) {
		std::string activeModes = channel->getActiveModes();
		std::string response = ":42_ft_IRC MODE " + channelName + " :" + activeModes + "\r\n";
		sendCMD(client->getFd(), response);
		return 0;
	}

	// Check if the client is an operator
	if (!channel->isOperator(client)) {
		#if DEBUG
		std::cout << "[DBG] Is operator" << std::endl;
        #endif
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(channel->getName()));
		return 0;
	}

	std::string modeChangeSummary = ":" + client->getNick() + " MODE " + channelName + " ";
	std::string paramSummary;

	bool adding = true;
	size_t i = 1;

	while (i < cmd.args.size()) {
		const std::string& token = cmd.args[i];

		if (token[0] == '+' || token[0] == '-') {
			for (size_t j = 0; j < token.size(); ++j) {
				char c = token[j];
				if (c == '+') {
					adding = true;
					modeChangeSummary += "+";
				} else if (c == '-') {
					adding = false;
					modeChangeSummary += "-";
				} else {
					switch (c) {
						case 'i':
							channel->setInviteOnly(adding);
							break;

						case 'o': {
							if (i + 1 >= cmd.args.size()) break;
							std::string targetNick = cmd.args[++i];
							Client* targetClient = getClientByNick(targetNick);
							if (!targetClient) break;
							if (adding)
								channel->addOperator(targetClient);
							else
								channel->removeOperator(targetClient);
							paramSummary += " " + targetNick;
							break;
						}

						case 'k': {
							if (adding) {
								if (i + 1 >= cmd.args.size()) break;
								std::string key = cmd.args[++i];
								channel->setKey(key);
								paramSummary += " " + key;
							} else {
								channel->setKey("");
							}
							break;
						}

						case 'l': {
							if (adding) {
								if (i + 1 >= cmd.args.size()) break;
								std::string limitStr = cmd.args[++i];
								int limit = std::atoi(limitStr.c_str());
								if (limit > 0)
									channel->setUserLimit(limit);
								paramSummary += " " + limitStr;
							} else {
								channel->setUserLimit(0);
							}
							break;
						}

						case 't': {
							channel->setTopicOnlyOps(adding);
							break;
						}

						default:
							sendCMD(client->getFd(), ERR_UNKNOWNMODE(std::string(1, c)));
							break;
					}
				}
			}
			++i;
		} else {
			++i;
		}
	}

	std::string msg = modeChangeSummary + paramSummary + "\r\n";
	const std::vector<Client*>& clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); ++i) {
		send(clients[i]->getFd(), msg.c_str(), msg.length(), 0);
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

    Channel* channel = getChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 " + client->getNick() + " " + channelName + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return 0;
    }

	if (!channel->isOperator(client)) {
		std::string err = ERR_CHANOPRIVSNEEDED(channelName);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

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
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command));
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
			sendCMD(client->getFd(), RPL_NOTOPIC(channelName));
		} else {
			sendCMD(client->getFd(), RPL_TOPIC(channelName, topic));
		}
		return 0;
	}

	if (channel->isTopicOnlyOps() && !channel->isOperator(client)) {
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(channelName));
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
		std::string err = ERR_NEEDMOREPARAMS(cmd.command);
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
		std::string err = ERR_CHANOPRIVSNEEDED(channelName);
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
		std::string err = ERR_NOSUCHNICK(targetNick);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Check if the target client is already in the channel
	if (channel->hasClient(targetClient)) {
		std::string err = ERR_USERONCHANNEL(targetNick ,channelName);
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	// Add the target client to the channel's invite list
	channel->inviteClient(targetNick);

	// Notify the target client about the invitation
	std::string inviteMsg = NTFY_CLIENTISINVITED(client->getNick(), targetNick, channel->getName());
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);

	// Notify the inviter that the invitation was successful
	std::string testMessage = RPL_INVITING(targetNick, channel->getName());
	send(client->getFd(), testMessage.c_str(), testMessage.length(), 0);

	return 0;
}