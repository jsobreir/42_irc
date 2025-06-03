#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel() 
	: _inviteOnly(false), _topicOnlyOps(true), _userLimit(0) 
{
	// _operators vector is default-initialized empty
}

Channel::Channel(int fd) 
	: _inviteOnly(false), _topicOnlyOps(true), _userLimit(0) 
{
	(void)fd;
}

Channel::Channel(Channel const &other) {
	_name = other._name;
	_channelClients = other._channelClients;
	_operators = other._operators;
	_topic = other._topic;
	_inviteOnly = other._inviteOnly;
	_topicOnlyOps = other._topicOnlyOps;
	_key = other._key;
	_userLimit = other._userLimit;

	std::cout << "Channel copy constructor called" << std::endl;
}

Channel &Channel::operator=(Channel const &other) {
	if (this != &other) {
		_name = other._name;
		_channelClients = other._channelClients;
		_operators = other._operators;
		_topic = other._topic;
		_inviteOnly = other._inviteOnly;
		_topicOnlyOps = other._topicOnlyOps;
		_key = other._key;
		_userLimit = other._userLimit;

		std::cout << "Channel assignment operator called" << std::endl;
	}
	return *this;
}

Channel::~Channel() {}

void Channel::setName(std::string name) {
	_name = name;
}

std::string Channel::getName() const {
	return _name;
}

void Client::setAuthenticated(bool value) {
	_authenticated = value;
}

bool Client::isAuthenticated() const {
	return _authenticated;
}

void Channel::addClient(Client* client) {
	if (!hasClient(client)) {
		_channelClients.push_back(client);
		std::cout << "Client " << client->getFd() << " joined channel " << _name << std::endl;

		if (_operators.empty()) {
			addOperator(client);
		}
	}
}

const std::vector<Client*> &Channel::getClients() const {
	return _channelClients;
}

void Channel::addOperator(Client* client) {
	if (!isOperator(client)) {
		_operators.push_back(client);
		std::cout << "Client " << client->getFd() << " granted operator in channel " << _name << std::endl;
	}
}

void Channel::removeOperator(Client* client) {
	for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
		if (*it == client) {
			_operators.erase(it);
			std::cout << "Client " << client->getFd() << " removed as operator in channel " << _name << std::endl;
			break;
		}
	}
}

bool Channel::isOperator(Client* client) const {
	for (std::vector<Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) {
		if (*it == client)
			return true;
	}
	return false;
}

std::string Channel::getTopic() const {
	return _topic;
}

void Channel::setTopic(const std::string& topic) {
	_topic = topic;
}

bool Channel::isInviteOnly() const {
	return _inviteOnly;
}

void Channel::setInviteOnly(bool enabled) {
	_inviteOnly = enabled;
}

bool Channel::isTopicOnlyOps() const {
	return _topicOnlyOps;
}

void Channel::setTopicOnlyOps(bool enabled) {
	_topicOnlyOps = enabled;
}

std::string Channel::getKey() const {
	return _key;
}

void Channel::setKey(const std::string& key) {
	_key = key;
}

size_t Channel::getUserLimit() const {
	return _userLimit;
}

void Channel::setUserLimit(size_t limit) {
	_userLimit = limit;
}

bool Channel::hasClient(Client* client) const {
    #if DEBUG
        std::cout << "[DBG - hasClient] Checking if there is a client " << std::endl;
    #endif
    for (size_t i = 0; i < _channelClients.size(); ++i) {
        #if DEBUG
            std::cout << "[DBG - hasClient] Clients in channel: " << _channelClients.size() << std::endl;
        #endif
        if (_channelClients[i] == client) {
            #if DEBUG
                std::cout << "[DBG - hasClient] Client found in channel." << std::endl;
            #endif
            return true;
        }
    }
    #if DEBUG
        std::cout << "[DBG - hasClient] Client not found in channel." << std::endl;
    #endif
    return false;
}

bool Channel::hasAnyClients() const {
    #if DEBUG
        std::cout << "[DBG - hasAnyClients] Checking if there are any clients in channel " << _name << std::endl;
        std::cout << "[DBG - hasAnyClients] Clients in channel: " << _channelClients.size() << std::endl;
    #endif
    return !_channelClients.empty();
}

void Channel::broadcastToChannel(const std::string& message) {
    for (std::vector<Client*>::iterator it = _channelClients.begin(); it != _channelClients.end(); ++it) {
        if (*it == NULL) {
            #if DEBUG
                std::cerr << "[DBG - broadcastToChannel] Null client in channel!" << std::endl;
            #endif
            continue;
        }
        (*it)->sendMessage(message);
    }
}

int Channel::removeClient(Client* client) {
	std::vector<Client*>::iterator it = std::find(_channelClients.begin(), _channelClients.end(), client);
	if (it != _channelClients.end()) {
		removeOperator(client); // remove operator status if any
		_channelClients.erase(it);
		std::cout << "Client " << client->getFd() << " left channel " << _name << std::endl;

		// ✅ Promote the next available client to operator if no operators remain
		// if (_operators.empty() && !_channelClients.empty()) {
		// 	Client* newOperator = _channelClients.front(); // pick first client
		// 	addOperator(newOperator);
		// 	std::cout << "Client " << newOperator->getFd() << " promoted to operator in channel " << _name << std::endl;
			
		// 	std::string modeMsg = ":" + _serverName_g + " MODE " + _name + " +o " + newOperator->getNick() + "\r\n";
		// 	broadcastToChannel(modeMsg);
		// }

		// Debug: SegFault - Print state of clients and operators
		#if DEBUG
        	std::cout << "Remaining clients in channel: " << _channelClients.size() << std::endl;
        	std::cout << "Operators in channel: " << _operators.size() << std::endl;
		#endif

		return 0;
	}
	return 1;
}

Client* Channel::getOperator() const {
    if (!_operators.empty())
        return _operators[0];
    return NULL;
}

bool Channel::isFull() const {
    return _userLimit > 0 && _channelClients.size() >= _userLimit;
}

bool Channel::isInvited(Client* client) const {
    return std::find(_invitedClients.begin(), _invitedClients.end(), client->getNick()) != _invitedClients.end();
}

void Channel::inviteClient(const std::string& nickname) {
    if (std::find(_invitedClients.begin(), _invitedClients.end(), nickname) == _invitedClients.end()) {
        _invitedClients.push_back(nickname);
    }
}

void Channel::revokeInvite(const std::string& nickname) {
    _invitedClients.erase(std::remove(_invitedClients.begin(), _invitedClients.end(), nickname), _invitedClients.end());
}

std::string Channel::getActiveModes() const {
    std::string modes = "+";

    if (_inviteOnly) {
        modes += "i"; // Invite-only mode
    }
    if (_topicOnlyOps) {
        modes += "t"; // Topic-only ops mode
    }
    if (!_key.empty()) {
        modes += "k"; // Key mode
    }
    if (_userLimit > 0) {
        modes += "l"; // User limit mode
    }

    return modes;
}