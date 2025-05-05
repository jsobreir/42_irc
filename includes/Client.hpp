#include "IRC.hpp"

class Client {
    public:
		Client ();
		Client (Client const &other);
		Client &operator=(Client const &other);
		~Client();
        void setPasswd(std::string passwd);
    private:
        int fd;
        bool _received_passwd;
        std::string _password;
} ;
