#ifndef SERVER_HPP
#define SERVER_HPP

#include "IRC.hpp"


class Client;
class Channel;

class Server {
	private:
		std::vector<Client *> _clients;
		std::vector<Channel *> _channels;
		int _nfds;
		int _server_fd;
		std::string _creationDate;
		std::string _serverName;
	public:
		Server ();
		Server (Server const &other);
		Server &operator=(Server const &other);
		~Server();
		int getServerFd(void) const;
		void start();
		void acceptNewClient(struct pollfd fds[]);
		void handleClientData(struct pollfd fds[]);
		int handleClientMessage(int client_fd, const char *msg);
		void closeAllClientFds(void);
		Client *getClient(int fd);
		Channel *getChannel(std::string channelName);
		void joinChannel(Client *client, const std::string &channelName);
};

void handleSIGINT(int sig);
#endif
