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
	_operators = other._operators;  // copy vector of operators
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

		// If no operators yet, make this client operator
		if (_operators.empty()) {
			addOperator(client);
		}
	}
}

const std::vector<Client*> &Channel::getClients() const {
	return _channelClients;
}

// Deprecated or remove these single operator methods since you have vector of operators:
// Client* Channel::getOperator() const { return nullptr; }
// void Channel::setOperator(Client* client) {}

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
	for (size_t i = 0; i < _channelClients.size(); ++i) {
		if (_channelClients[i] == client)
			return true;
	}
	return false;
}

void Channel::removeClient(Client* client) {
	for (std::vector<Client*>::iterator it = _channelClients.begin(); it != _channelClients.end(); ++it) {
		if (*it == client) {
			_channelClients.erase(it);
			std::cout << "Client " << client->getFd() << " left channel " << _name << std::endl;
			removeOperator(client); // remove operator status if any
			break;
		}
	}
}

Client* Channel::getOperator() const {
    if (!_operators.empty())
        return _operators[0];
    return NULL; // or nullptr in C++11+
}

#include <fnmatch.h> // POSIX wildcard matching

bool Channel::isBanned(Client* client) const {
	// Check basic nickname ban
	if (std::find(_bannedClients.begin(), _bannedClients.end(), client->getNick()) != _bannedClients.end())
		return true;

	// Build full identity: nick!user@host
	std::string identity = client->getNick() + "!" + client->getUser() + "@localhost"; // change host if needed

	// Check ban masks with wildcard support
	for (size_t i = 0; i < _banMasks.size(); ++i) {
		if (fnmatch(_banMasks[i].c_str(), identity.c_str(), 0) == 0)
			return true;
	}

	return false;
}

void Channel::addBanMask(const std::string& mask) {
	if (std::find(_banMasks.begin(), _banMasks.end(), mask) == _banMasks.end()) {
		_banMasks.push_back(mask);
	}
}

void Channel::removeBanMask(const std::string& mask) {
	_banMasks.erase(std::remove(_banMasks.begin(), _banMasks.end(), mask), _banMasks.end());
}

const std::vector<std::string>& Channel::getBanMasks() const {
	return _banMasks;
}


void Channel::banClient(const std::string& nickname) {
    if (std::find(_bannedClients.begin(), _bannedClients.end(), nickname) == _bannedClients.end()) {
        _bannedClients.push_back(nickname);
    }
}

void Channel::unbanClient(const std::string& nickname) {
    _bannedClients.erase(std::remove(_bannedClients.begin(), _bannedClients.end(), nickname), _bannedClients.end());
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
