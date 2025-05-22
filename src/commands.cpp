#include "Server.hpp"

int Server::handleClientMessage(int fd, const char *msg) {
    std::stringstream ss(msg);
	std::string line;

	// Process each line (command) separately
	while (std::getline(ss, line)) {
		// Remove any trailing carriage return
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		std::stringstream commandStream(line);
		std::string command;
		commandStream >> command;

		Client *client = getClient(fd);
		if (!client)
			return 0;
    
    int n_commands = 7;
    std::string commands[n_commands] = {"CAP", "PASS", "NICK", "USER", "JOIN", "QUIT", "PRIVMSG"};
    int i;
    for (i = 0; i < n_commands; i++) {
        if (command == commands[i])
            break;
    }
    switch (i)
    {
        case 0:
            handleCapCMD();
            break;
        case 1:
            handlePassCMD();
            break;
        case 2:
            handleNickCMD();
            break;
        case 3:
            handleUserCMD();
            break;
        case 4:
            handleJoinCMD();
            break;
        case 5:
            handleQuitCMD();
            break;
        case 6:
            handlePrivMsgCMD();
            break;
        default:
            #if DEBUG
            std::cout << "[DBG]Unknown command: " << command << std::endl;
            #endif   
            break;
    }
}

handleCapCMD();
