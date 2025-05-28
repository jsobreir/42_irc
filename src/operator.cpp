#include "IRC.hpp"

int Server::handleModeOperatorCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.empty()) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command));
		return 0;
	}

	std::string channelName = cmd.args[0];
	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
		return 0;
	}

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

	std::string finalMsg = modeChangeSummary + paramSummary + "\r\n";
	const std::vector<Client*>& clients = channel->getClients();
	for (size_t j = 0; j < clients.size(); ++j) {
		send(clients[j]->getFd(), finalMsg.c_str(), finalMsg.length(), 0);
	}

	return 0;
}



int Server::handleKickOperatorCMD(IRCCommand cmd, Client *client) {
    // USAGE: /KICK <channel> <nickname> [:reason...]
    if (cmd.args.size() < 2) {
        std::string err = ":server 461 " + client->getNick() + " KICK :Not enough parameters\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
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
        std::string err = ":server 482 " + client->getNick() + " " + channelName + " :You're not channel operator\r\n";
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
        std::string err = ":server 441 " + client->getNick() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
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
	if (cmd.args.size() < 2) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command));
		return 1;
	}
	Channel *channel = getChannel(cmd.args[1]);
	if (channel == NULL) {
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(cmd.args[1]));
		return 1;
	}
	if (channel->hasClient(client) == false) {
		sendCMD(client->getFd(), ERR_NOTONCHANNEL(client->getNick(), channel->getName()));
		return 1;
	}
	if (channel->isInviteOnly() && !channel->isOperator(client)) {
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(channel->getName()));
		return 1;	
	}
	Client *targetClient = getClientByNick(cmd.args[0]);
	if (!targetClient) {
		sendCMD(client->getFd(), ERR_NOSUCHNICK(cmd.args[0]));
		return 1;
	}
	if (channel->hasClient(targetClient)) {
		sendCMD(client->getFd(), ERR_USERONCHANNEL(targetClient->getNick(), channel->getName()));
		return 1;	
	}
	sendCMD(client->getFd(), RPL_INVITING(targetClient->getNick(), channel->getName()));
	std::string msg = ":" + client->getNick() + "!" + client->getUser() + "@localhost INVITE " + targetClient->getNick() + " :" + channel->getName() + "\r\n";

	sendCMD(targetClient->getFd(), msg);
	joinChannel(targetClient, channel->getName());
	return 0;
}
