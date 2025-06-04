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
		
		std::string getCreationDate() const;
		std::string getServerName() const;
		int 	getServerFd(void) const;
		Client 	*getClient(int fd);
		Client 	*getClientByNick(const std::string &nickname);
		Channel *getChannel(std::string channelName);
		const 	std::string &getPassword() const;
		void 	start();
		void 	acceptNewClient(struct pollfd fds[]);
		void 	handleClientData(struct pollfd fds[]);
		int 	handleClientMessage(int client_fd);
		void 	sendCMD(int fd, std::string msg);
		void 	closeAllClientFds(void);
		void 	removeClient(Client *client);
		void 	joinChannel(Client *client, const std::string &channelName);

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
		int		handlePingCMD(IRCCommand cmd, Client *client);
		int 	handlePartCMD(IRCCommand cmd, Client *client);
		void	broadcastMsg(Channel *channel, std::string msg, Client *client);
		bool	isValidNickname(const std::string &nick);

		void	clear_data();
};

#endif
