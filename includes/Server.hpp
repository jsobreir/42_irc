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
		int _port;
		std::string _password;
		std::string _serverVersion;
	public:
		Server ();
		Server (int port, std::string password);
		Server (Server const &other); 
		Server &operator=(Server const &other);
		~Server();
		int getServerFd(void) const;
		void start();
		void acceptNewClient(struct pollfd fds[]);
		void handleClientData(struct pollfd fds[]);
		int handleClientMessage(int client_fd, const char *msg);
		void sendCMD(int fd, std::string msg);
		void closeAllClientFds(void);
		Client *getClient(int fd);
		void 	removeClient(Client *client);
		Client *getClientByNick(const std::string &nickname);
		Channel *getChannel(std::string channelName);
		void 	joinChannel(Client *client, const std::string &channelName);
		const std::string &getPassword() const;
		int 	handleCapCMD(IRCCommand cmd, Client *client);
		int 	handlePassCMD(IRCCommand cmd, Client *client);
		int 	handleNickCMD(IRCCommand cmd, Client *client);
		int 	handleUserCMD(IRCCommand cmd, Client *client);
		int		handleJoinCMD(IRCCommand cmd, Client *client);
		int 	handleQuitCMD(IRCCommand cmd, Client *client);
		int 	handlePrivMsgCMD(IRCCommand cmd, Client *client);
		int 	handleModeOperatorCMD(IRCCommand cmd, Client *client);
		int		handleKickOperatorCMD(IRCCommand cmd, Client *client);
		int		handleInviteOperatorCMD(IRCCommand cmd, Client *client);
		int		handleTopicOperatorCMD(IRCCommand cmd, Client *client);
};

void 	handleSIGINT(int sig);

#endif
