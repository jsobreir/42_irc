#include "IRC.hpp"

int Server::handleModeOperatorCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.empty()) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(client->getNick(), cmd.command));
		#if DEBUG
			std::cout << "Entered No channel name if" << std::endl;
		#endif
		return 0;
	}

	std::string channelName = cmd.args[0];
	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(client->getNick(), channelName));
		return 0;
	}

	if (cmd.args.size() == 1) {
		std::string activeModes = channel->getActiveModes();
		std::string response = ":42_ft_IRC MODE " + channelName + " :" + activeModes + "\r\n";
		sendCMD(client->getFd(), response);
		return 0;
	}

	if (!channel->isOperator(client)) {
		#if DEBUG
		std::cout << "[DBG] Is operator" << std::endl;
		#endif
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(client->getNick(), channel->getName()));
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
							modeChangeSummary += "i";
							channel->setInviteOnly(adding);
							break;

						case 't':
						modeChangeSummary += "t";
						channel->setTopicOnlyOps(adding);
						break;

						case 'k':
							modeChangeSummary += "k";
							if (adding) {
								if (i + 1 >= cmd.args.size()) break;
								std::string key = cmd.args[++i];
								channel->setKey(key);
								paramSummary += " " + key;
							} else {
								channel->setKey("");
							}
							break;
						
						case 'o':
							modeChangeSummary += "o";
							if (i + 1 >= cmd.args.size()) break;
							{
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

						case 'l':
							modeChangeSummary += "l";
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

						default:
							sendCMD(client->getFd(), ERR_UNKNOWNMODE(client->getNick(), std::string(1, c)));
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
	if (cmd.args.size() < 2) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(client->getNick(), cmd.command));
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
		std::string err = ERR_CHANOPRIVSNEEDED(client->getNick(), channelName);
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

	std::string kickMsg = ":" + client->getNick() + " KICK " + channelName + " " + targetNick;
	if (!reason.empty())
		kickMsg += " :" + reason;
	kickMsg += "\r\n";

	for (size_t i = 0; i < clients.size(); ++i)
		send(clients[i]->getFd(), kickMsg.c_str(), kickMsg.length(), 0);

	send(targetClient->getFd(), kickMsg.c_str(), kickMsg.length(), 0);

	channel->removeClient(targetClient);
	targetClient->decrementJoinedChannels();
	return 0;
}

int Server::handleTopicOperatorCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.empty()) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(client->getNick(), cmd.command));
		return 0;
	}

	std::string channelName = cmd.args[0];
	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(client->getNick(), channelName));
		return 0;
	}

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

	std::string newTopic;
	for (size_t i = 1; i < cmd.args.size(); ++i) {
		if (i > 1)
			newTopic += " ";
		newTopic += cmd.args[i];
	}

	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic = newTopic.substr(1);

	channel->setTopic(newTopic);

	std::string topicMsg = ":" + client->getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	const std::vector<Client*>& clients = channel->getClients();
	for (size_t i = 0; i < clients.size(); ++i) {
		send(clients[i]->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
	}

	return 0;
}

int		Server::handleInviteOperatorCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.size() < 2) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(client->getNick(), cmd.command));
		return 1;
	}
	Channel *channel = getChannel(cmd.args[1]);
	if (channel == NULL) {
		sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(cmd.args[1], channel->getName()));
		return 1;
	}
	if (channel->hasClient(client) == false) {
		sendCMD(client->getFd(), ERR_NOTONCHANNEL(client->getNick(), channel->getName()));
		return 1;
	}
	if (channel->isInviteOnly() && !channel->isOperator(client)) {
		sendCMD(client->getFd(), ERR_CHANOPRIVSNEEDED(client->getNick(), channel->getName()));
		return 1;	
	}
	Client *targetClient = getClientByNick(cmd.args[0]);
	if (!targetClient) {
		sendCMD(client->getFd(), ERR_NOSUCHNICK(client->getNick(), cmd.args[0]));
		return 1;
	}
	if (channel->hasClient(targetClient)) {
		sendCMD(client->getFd(), ERR_USERONCHANNEL(targetClient->getNick(), channel->getName()));
		return 1;	
	}
	sendCMD(client->getFd(), RPL_INVITING(client->getNick(), targetClient->getNick(), channel->getName()));
	std::string msg = ":" + client->getNick() + "!" + client->getUser() + "@localhost INVITE " + targetClient->getNick() + " :" + channel->getName() + "\r\n";

	sendCMD(targetClient->getFd(), msg);
	channel->inviteClient(targetClient->getNick());

	return 0;
}