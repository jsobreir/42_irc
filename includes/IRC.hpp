#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fnmatch.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

struct IRCCommand {
	std::string command;
	std::vector<std::string> args;
} ;

IRCCommand  parseIRCLine(const std::string &line);
void        cleanup_and_exit(int signum);

#include "Messages.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server;
extern Server *g_server;

#define MAX_CLIENTS 1024
#define MAX_CHANNELS 1024
#define DEBUG 0