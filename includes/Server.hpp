#ifndef SERVER_HPP
#define SERVER_HPP

#include "IRC.hpp"


class Client;
class Channel;

class Server {
	private:
		std::vector<Client *> _clients;
		std::vector<Channel> _channels;
		std::string _creationDate;
		std::string _serverName;
		std::string _serverVersion;
	public:
		Server ();
		Server (Server const &other);
		Server &operator=(Server const &other);
		~Server();
		void start();
		int handleClientMessage(int client_fd, const char *msg);
		Client *getClient(int fd);
		Channel *getChannel(std::string channelName);
		void joinChannel(Client *client, const std::string &channelName);
};

#endif
