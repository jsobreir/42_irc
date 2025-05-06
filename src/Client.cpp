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

void Client::setPasswd(std::string passwd) {
    _password = passwd;
    _received_passwd = true;
}
