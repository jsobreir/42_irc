#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "IRC.hpp"

class Channel {
    public:
		Channel ();
		Channel (int fd);
		Channel (Channel const &other);
		Channel &operator=(Channel const &other);
		~Channel();
    private:
        std::vector<Client *> _channelClients;
} ;

#endif
