#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "IRC.hpp"

class Channel;

class Client {
    public:
		Client ();
		Client (int fd);
		Client (Client const &other);
		Client &operator=(Client const &other);
		int getFd() const;
		void setPass(const std::string &pass);
		~Client();
        void setPasswd(std::string passwd);
		void setNick(std::string nick);
		void setUser(std::string user);
		std::string getNick() const;
		std::string getUser() const;
    private:
        int _fd;
        bool _received_passwd;
        std::string _password;
		std::string _nick;
		std::string _user;
		//Channel	*_channel;
} ;


#endif
