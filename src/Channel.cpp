#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel () : _operator(NULL) {}

Channel::Channel (int fd) : _operator(NULL) {
    (void)fd;
}

Channel::Channel (Channel const &other) {
    _name = other._name;
    _channelClients = other._channelClients;
    _operator = other._operator;
    std::cout << "Channel copy constructor called" << std::endl;
}

Channel &Channel::operator=(Channel const &other) {
    if (this != &other) {
        _name = other._name;
        _channelClients = other._channelClients;
		_operator = other._operator;
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

void Channel::addClient(Client *client) {
    _channelClients.push_back(client);
    if (!_operator) {
        _operator = client;
        std::cout << "Client " << client->getFd() << " set as operator in channel " << _name << std::endl;
    }
    std::cout << "Client " << client->getFd() << " joined channel " << _name << std::endl;
}

const std::vector<Client*> &Channel::getClients() const {
    return _channelClients;
}

Client* Channel::getOperator() const {
    return _operator;
}

void Channel::setOperator(Client* client) {
    _operator = client;
}