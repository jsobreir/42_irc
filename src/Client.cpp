#include "Client.hpp"

Client::Client () {

}

Client::Client (int fd) {
	_fd = fd;
}

Client::Client (Client const &other) {
	(void)other;
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
