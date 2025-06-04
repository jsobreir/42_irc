#include "Client.hpp"

Client::Client () : _joinedChannels(0){

}

Client::Client (int fd) : _joinedChannels(0){
    _fd = fd;
    _joinedChannels = 0;
	_authenticated = false;
}

Client::Client(Client const &other) {

    _fd = other._fd;
    _nick = other._nick;
    _user = other._user;
    _joinedChannels = other._joinedChannels;
	_authenticated = other._authenticated;
}

int Client::getFd() const {
    return _fd;
}

Client &Client::operator=(Client const &other) {
	(void)other;
	return *this;
}

Client::~Client() {

}

void Client::setNick(std::string nick)
{
	_nick = nick;
	std::cout << "Client " << _fd << " nick is: " << _nick << std::endl;
}

void Client::setUser(std::string user)
{
	_user = user;
	std::cout << "Client " << _fd << " user is: " << _user << std::endl;
}

std::string Client::getNick() const
{
	return _nick;
}

std::string Client::getUser() const
{
	return _user;
}

std::string Client::getBuffer() const {
	return _clientBuffer;
}

void Client::appendBuffer(std::string buffer){
	_clientBuffer.append(buffer);
}

void Client::setBuffer(std::string buffer) {
	_clientBuffer = buffer;
}

void Client::incrementJoinedChannels()
{
	_joinedChannels++;
}

void Client::decrementJoinedChannels()
{
	_joinedChannels--;
}

size_t Client::getChannelCount() const
{
	return _joinedChannels;
}

void Client::sendMessage(const std::string& message) {
    send(_fd, message.c_str(), message.size(), 0);
}