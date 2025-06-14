#include "IRC.hpp"

IRCCommand parseIRCLine(const std::string &line) {
	IRCCommand result;
	std::istringstream ss(line);
	ss >> result.command;
	std::string token;

	while (ss >> token) {
		if (token[0] == ':') {
			std::string trailing = token.substr(1);
			std::string rest;
			std::getline(ss, rest);
			trailing += rest;
			result.args.push_back(trailing);
			break;
		}
		result.args.push_back(token);
	}
	return result;
}

void cleanup_and_exit(int signum) {
	g_server->closeAllClientFds();
	if (g_server) {
		delete g_server;
		g_server = NULL;
	}
	std::cout << "\nServer shutting down CTRL+C was pressed " << signum << std::endl;
	std::exit(EXIT_SUCCESS);
}