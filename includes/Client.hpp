#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "IRC.hpp"

class Channel;

class Client {
	public:
		Client();
		Client(int fd);
		Client(Client const &other);
		Client &operator=(Client const &other);
		~Client();

		int getFd() const;
		void setNick(std::string nick);
		void setUser(std::string user);
		std::string getNick() const;
		std::string getUser() const;
		std::string getBuffer() const;
		void appendBuffer(std::string buffer);
		void setBuffer(std::string buffer);
		void setAuthenticated(bool value);
		bool isAuthenticated() const;
		size_t getChannelCount() const;
		void incrementJoinedChannels();
		void decrementJoinedChannels();
		void sendMessage(const std::string& message);


	private:
		int _fd;
		std::string _nick;
		std::string _user;
		bool _authenticated; 
		size_t _joinedChannels;
		std::string _clientBuffer;
};

#endif
