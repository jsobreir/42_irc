#ifndef IRC_HPP
#define IRC_HPP

#include <errno.h>
#include <iostream>
#include <vector>
#include <cstring>     // For memset
#include <unistd.h>    // For close()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For htons, inet_*
#include <poll.h>      // For struct pollfd, poll()
#include <cstdlib>     // For perror
#include <cstdio> 
#include <sstream>
#include <algorithm>
#include <vector>
#include <csignal>
#include <ctime>
#include <sstream>

#include <Messages.hpp>
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server;
extern Server *g_server;

#define MAX_CLIENTS 1024
#define DEBUG 0

#endif