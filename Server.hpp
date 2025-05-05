#iniclude <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

public class Server
{
	private:
		int server_fd;
		int client_fd;
		struct sockaddr_in server_addr, client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char buffer[1024];
		int port = 6667; // Default IRC port

		public:
		Server();
		~Server();
		void init();
		void acceptConnection();
		void readData();
		void sendData(const std::string &data);
		void closeConnection();
		void handleClient();
		void handleCommand(const std::string &command);
		void handleJoin(const std::string &channel);
		void handlePart(const std::string &channel);
		void handlePrivmsg(const std::string &target, const std::string &message);
		void handleNick(const std::string &nickname);
		void handleUser(const std::string &username, const std::string &hostname, const std::string &servername, const std::string &realname);
		void handlePing(const std::string &server);
		void handlePong(const std::string &server);
		void handleMode(const std::string &target, const std::string &mode);
		void handleTopic(const std::string &channel, const std::string &topic);
		void handleNames(const std::string &channel);
		void handleWho(const std::string &channel);
		void handleList(const std::string &channel);
		void handleInvite(const std::string &channel, const std::string &nickname);
		void handleKick(const std::string &channel, const std::string &nickname, const std::string &reason);
		void handleMotd();
		void handleLusers();
		void handleVersion();
		void handleTime();
		void handleAdmin();
		void handleInfo();
		void handleWhois(const std::string &nickname);
		void handleWhowas(const std::string &nickname);
		void handleKill(const std::string &nickname, const std::string &reason);
}