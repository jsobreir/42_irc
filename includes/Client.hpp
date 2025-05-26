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
		void setAuthenticated(bool value);
		bool isAuthenticated() const;
		size_t getChannelCount() const;
		void incrementJoinedChannels();
		void decrementJoinedChannels();


	private:
		int _fd;
		std::string _nick;
		std::string _user;
		bool _authenticated; 
		size_t _joinedChannels;
};

#endif
