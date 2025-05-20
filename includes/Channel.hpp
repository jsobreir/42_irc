#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "IRC.hpp"

class Client;

class Channel {
    public:
		Channel ();
		Channel (int fd);
		Channel (Channel const &other);
		Channel &operator=(Channel const &other);
		~Channel();
		std::string getName() const;
		void setName(std::string name);
		void addClient(Client *client);
    private:
        std::vector<Client *> _channelClients;
		std::string _name;
} ;

#endif
