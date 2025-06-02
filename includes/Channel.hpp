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
		bool hasAnyClients() const;
		int removeClient(Client* client);

        //bool isBanned(Client* client) const;
        //void banClient(const std::string& nickname);
        //void unbanClient(const std::string& nickname);

		//const std::vector<std::string>& getBanMasks() const;
		//void addBanMask(const std::string& mask);
		//void removeBanMask(const std::string& mask);

        bool isFull() const;

        bool isInvited(Client* client) const;
        void inviteClient(const std::string& nickname);
        void revokeInvite(const std::string& nickname);

		void broadcastToChannel(const std::string& message);

    private:
		std::vector<std::string> _banMasks;
		std::vector<Client *> _channelClients;
		std::vector<Client*> _operators;
		std::vector<std::string> _bannedClients;
		std::vector<std::string> _invitedClients;
		std::string _name;
		std::string _topic;
		bool _inviteOnly;
		bool _topicOnlyOps;
		std::string _key;
		size_t _userLimit;
} ;

#endif
