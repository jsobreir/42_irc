#pragma once

/*------Connection replies-----*/
#define RPL_WELCOME(nick, network) ("001 " + client->getNick() + " :Welcome to "+ _serverName + ", " + client->getNick() + "\n")

#define RPL_YOURHOST(server) ("002 :Your host is " + _serverName + ", running version 1.0\n")

#define RPL_CREATED(date) ("003 :This server was created " + _creationDate + "\n")

#define RPL_MYINFO(server, nick, version) ("004 " + nick + " " + server + " " + version + " :Available user modes: io, channel modes: tkl\r\n")

/*-----------------------------*/


#define RPL_JOIN(nick, user,host, channel) (":" + client->getNick() + "!" + user + "@" + host + " JOIN " + channel + "\r\n")


/*-------Channel replies------ */
#define RPL_ENDOFWHO(nick, mask) (":315 " + client->getNick() + " " + mask + " :End of WHO list\r\n")

#define RPL_LISTSTART(channel, user, name) ("321 " + channel + "Users " + name + "\r\n")
#define RPL_LIST "322\r\n"
#define RPL_LISTEND "323\r\n"

#define RPL_CHANNELMODEIS(channel, mode, params) ("324 " + channel + " " + mode + " " + params + "\n")

#define RPL_NOTOPIC(channel) ("331 " + channel + " :No topic is set\n")

#define RPL_TOPIC(channel, topic) ("332" + channel + " :" + topic + "\r\n")

#define RPL_TOPIC2(nickname, channel, topic) ("332 " + nickname + " " + channel + " :" + topic + "\r\n")

#define RPL_TOPICWHOTIME(channel, nick, time) (":" + _serverName + " 333 " + client->getNick() + " " + channelName + " " + time + "\r\n")

#define RPL_INVITING(nick, channel) ("341" + client->getNick() + " " + channel + "\r\n")

#define RPL_WHOREPLY(nickname, host,  channelname, user, realname, flag) (":" + host + " 352 " + client->getNick() + " " + channelname + " " + host + " " + SERVER_NAME + " " + user + " " + flag + " :2 " + realname + "\r\n")

#define RPL_NAMREPLY(nick, channel, users) (":" + _serverName + " 353 " + nick + " = " + channel + " :" + users + "\r\n")

#define RPL_ENDOFNAMES(channel) ("366 " + channelName + " End of /NAMES list\r\n")
/*-----------------------------*/


/*-------Miscellaneous---------*/
#define RPL_MOTDSTART(nick) "375 " + client->getNick() + " - " + _serverName + " Message of the day - \r\n"
#define RPL_MOTD(nick) "372 " + client->getNick() + "  Welcome to " + _serverName + ", and remember what happens in " + _serverName + " stays in " + _serverName + " 😎.\r\n"
#define RPL_ENDOFMOTD(nick) "376 " + client->getNick() + "End of /MOTD command.\r\n"

/*-----------------------------*/


/*---------Error replies-------*/
#define ERR_NOSUCHNICK(nick) ("401 " + client->getNick() + " :No such nick/channel\n")

#define ERR_NOSUCHCHANNEL(channel) ("403 " + channel + " :No such channel\n")

#define ERR_TOOMANYCHANNELS(channel) (":server 405 " + client->getNick() + " " + channel + "You have joined too many channels\r\n")

#define ERR_UNKNOWNCOMMAND(command) ("421 " + command + " :Unknown command\n")

#define ERR_NONICKNAMEGIVEN "431 :No nickname given\n"

#define ERR_NICKNAMEINUSE(nick) (": 433 * " + client->getNick() + " :Nickname is already in use\r\n")

#define ERR_ERRONEUSNICKNAME(nick) "432" + client->getNick() + " :Erroneus nickname\n"

#define ERR_NICKCOLLISION(nick) ("436 " + client->getNick() + " :Nickname collision KILL\n")

#define ERR_NOTONCHANNEL(nick, channel) ("442 " + client->getNick() + " " + channel + " :You're not on that channel\n")

#define ERR_USERONCHANNEL(nick, channel) ("443 " + client->getNick() + " " + channel + " :is already on channel\n")

#define ERR_NEEDMOREPARAMS(command) ("461 " + command + " :Not enough parameters\n")

#define ERR_ALREADYREGISTRED "462 :You may not reregister\n"

#define ERR_PASSWDMISMATCH "464 :Password incorrect\n"

#define ERR_CHANNELISFULL(channelName) ("471 " + client->getNick() + channelName + " :Cannot join channel (+l)\n")

#define ERR_UNKNOWNMODE(mode) ("472 " + mode + " :is unknown mode char to me\n")

#define ERR_INVITEONLYCHAN(channelName) ("473 " + client->getNick() + channelName + " :Cannot join channel (+b)\n")

#define ERR_BANNEDFROMCHAN(channelName) ("474 " + client->getNick() + channelName + " :Cannot join channel (+b)\n")

#define ERR_BADCHANMASK(channel) ("476 " + channelName + " :Invalid channel name\r\n")

//#define ERR_CHANOPRIVSNEEDED(channel) ("482 " + channel + " :You're not channel operator\n")
#define ERR_CHANOPRIVSNEEDED(client, channel) (":" + _serverName + " 482 " + client + " " + channel + " :You're not channel operator\r\n")
/*-----------------------------*/
