#include "Client.hpp"

Client::Client () {

}

Client::Client (Client const &other) {

}

Client &Client::operator=(Client const &other) {

}

Client::~Client() {

}

void Client::setPasswd(std::string passwd) {
    _passwd = passwd;
    _received_passwd = true;
}
