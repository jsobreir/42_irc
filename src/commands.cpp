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
	if (cmd.args.size()) {
		std::string key = cmd.args[0];
		if (key != _password) {
			sendCMD(client->getFd(), ERR_PASSWDMISMATCH);
			sendCMD(client->getFd(), "ERROR :Wrong Password");
			int fd = client->getFd();
			close(fd);
		}
	}
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
		std::string welcome = RPL_WELCOME(client->getNick(), getServerName());
		send(client->getFd(), welcome.c_str(), welcome.length(), 0);

		std::string hostInfo = RPL_YOURHOST(getServerName());
		send(client->getFd(), hostInfo.c_str(), hostInfo.length(), 0);

		std::string creationDate = RPL_CREATED(getCreationDate());
		send(client->getFd(), creationDate.c_str(), creationDate.length(), 0);

		std::string capabilities = RPL_MYINFO(getServerName(), client->getNick(), _serverVersion);
		send(client->getFd(), capabilities.c_str(), capabilities.length(), 0);

		std::string motdStart = RPL_MOTDSTART(client->getNick(), getServerName());
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
		if (channelName[0] != '#') {
			sendCMD(client->getFd(), ERR_BADCHANMASK(channelName));
			continue;
		}

		std::string key;
		if (cmd.args.size() > 2) {
			std::stringstream buf(cmd.args[2]);
			if (std::getline(buf, key, ',')) {
			}
		}


		if (channel && channel->isFull()) {
			sendCMD(client->getFd(), ERR_CHANNELISFULL(client->getNick(), channelName));
			continue;
		}

		if (channel && channel->isInviteOnly() && !channel->isInvited(client)) {
			sendCMD(client->getFd(), ERR_INVITEONLYCHAN(client->getNick(), channelName));
			continue;
		}

		if (client->getChannelCount() >= MAX_CHANNELS) {
			sendCMD(client->getFd(), ERR_TOOMANYCHANNELS(client->getNick(), channelName));
			continue;
		}

		#if DEBUG
			std::cout << "[DBG] Joining channel. " << std::endl;	
		#endif
		joinChannel(client, channelName);
		#if DEBUG
			std::cout << "[DBG]Client " << client->getFd() << " joined channel " << channelName << std::endl;
		#endif
	}
	return 0;
}

int Server::handleQuitCMD(IRCCommand cmd, Client *client) {
    #if DEBUG
        std::cout << "[DBG]Client " << client->getFd() << " disconnected" << std::endl;
    #endif

    std::string reason;
    if (!cmd.args.empty()) {
        for (size_t i = 0; i < cmd.args.size(); ++i) {
            reason.append(cmd.args[i]);
            if (i != cmd.args.size() - 1)
                reason.append(" ");
        }
    }

    std::string quitMsg = ":" + client->getNick() + "!" + client->getUser() + "@127.0.0.1 QUIT :" + reason + "#7\r\n";

    for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        Channel* chan = *it;

        // Send QUIT to other clients in the channel *before* removing the client
        if (chan->hasClient(client)) {
			broadcastMsg(chan, quitMsg, client);
            chan->removeClient(client);
        }
    }

    close(client->getFd());
    removeClient(client);
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
			sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(target));
			return 1;
		}
		
		if (channel->hasClient(client) == false) {
			#if DEBUG
				std::cout << "[DBG] PRIVMSG - Client not on channel." << std::endl;
			#endif
			sendCMD(client->getFd(), ERR_CANNOTSENDTOCHAN(client->getNick(), channel->getName()));
			return 1;
		}
		// Construct the full message
		std::string fullMsg = ":" + client->getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
		broadcastMsg(channel, fullMsg, client);
	}
	return 0;
}

int Server::handlePingCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.empty()) {
		std::string err = ":server 409 " + client->getNick() + " :No origin specified\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return 0;
	}

	std::string token = cmd.args[0];
	std::string response = "PONG :" + token + "\r\n";
	send(client->getFd(), response.c_str(), response.length(), 0);

	#if DEBUG
	std::cout << "[PING] Received from " << client->getNick() << " token: " << token << std::endl;
	std::cout << "[PING] Replied with: " << response;
	#endif

	return 0;
}

int 	Server::handlePartCMD(IRCCommand cmd, Client *client) {
	if (cmd.args.size() < 2) {
		sendCMD(client->getFd(), ERR_NEEDMOREPARAMS(cmd.command));
		return 1;
	}
	std::string channelName;
	std::stringstream ss(cmd.args[0]);
	while (std::getline(ss, channelName, ',')) {
		Channel *channel = getChannel(channelName);
		if (channel == NULL) {
			sendCMD(client->getFd(), ERR_NOSUCHCHANNEL(channelName));
			return 1;
		}
		if (channel->removeClient(client) == 1) {
			sendCMD(client->getFd(), ERR_NOTONCHANNEL(client->getNick(), channelName));
			return 1;
		}
		std::string reason;
		for (size_t i = 1; i < cmd.args.size(); i++) {
			reason.append(cmd.args[i]);
		}
		std::string partMsg = ":" + client->getNick() + "!" + client->getUser() + "@localhost PART " + channelName + " :" + reason + "\r\n";
		broadcastMsg(channel, partMsg, client);
		sendCMD(client->getFd(), partMsg);
	}
	return 0;
}
