#include "Client.hpp"

Client::Client () : _joinedChannels(0){

}

Client::Client (int fd) : _joinedChannels(0){
	_fd = fd;
}

Client::Client(Client const &other) {

    _fd = other._fd;
    _received_passwd = other._received_passwd;
    _password = other._password;
    _nick = other._nick;
    _user = other._user;
    _joinedChannels = other._joinedChannels;
}

int Client::getFd() const {
    return _fd;
}

void Client::setPass(const std::string &pass) {
    _password = pass;
}
Client &Client::operator=(Client const &other) {
	(void)other;
	return *this;
}

Client::~Client() {

}

void Client::setPasswd(std::string passwd)
{
    if (passwd.length() > 5)
        _password = passwd.substr(5);
    else
        _password = "";
    _received_passwd = true;
	std::cout << "Client " << _fd << " passwd is: " << _password << std::endl;
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