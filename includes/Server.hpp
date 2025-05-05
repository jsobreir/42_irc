#include "IRC.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include "Client.hpp"

class Server {
	private:
		std::vector<Client *> _clients;
	public:
		Server ();
		Server (Server const &other);
		Server &operator=(Server const &other);
		~Server();
		void start();
		void handleClientMessage(int client_fd, const char *msg);
}
