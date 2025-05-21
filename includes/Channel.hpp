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
		const std::vector<Client*>& getClients() const;
		Client* getOperator() const;
		void setOperator(Client* client);
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool isOperator(Client* client) const;

		std::string getTopic() const;
		void setTopic(const std::string& topic);
	
		bool isInviteOnly() const;
		void setInviteOnly(bool enabled);
	
		bool isTopicOnlyOps() const;
		void setTopicOnlyOps(bool enabled);
	
		std::string getKey() const;
		void setKey(const std::string& key);
	
		size_t getUserLimit() const;
		void setUserLimit(size_t limit);
	
		bool hasClient(Client* client) const;
		void removeClient(Client* client);
    private:
    std::vector<Client *> _channelClients;
    std::string _name;
    std::vector<Client*> _operators;
    std::string _topic;
    bool _inviteOnly;
    bool _topicOnlyOps;
    std::string _key;
    size_t _userLimit;
} ;

#endif
