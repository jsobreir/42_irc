#pragma once

/* Global server name accessor */
#define SERVER_NAME g_server->getServerName()

/*------Connection replies-----*/
#define RPL_WELCOME(nick, network) (":" + SERVER_NAME + " 001 " + nick + " :Welcome to " + network + ", " + nick + "\r\n")
#define RPL_YOURHOST(nick) (":" + SERVER_NAME + " 002 " + nick + " :Your host is " + SERVER_NAME + ", running version 1.0\r\n")
#define RPL_CREATED(nick, date) (":" + SERVER_NAME + " 003 " + nick + " :This server was created " + date + "\r\n")
#define RPL_MYINFO(nick, version) (":" + SERVER_NAME + " 004 " + nick + " " + SERVER_NAME + " " + version + " :Available user modes: io, channel modes: tkl\r\n")

/*------Join message------*/
#define RPL_JOIN(nick, user, host, channel) (":" + nick + "!" + user + "@" + host + " JOIN " + channel + "\r\n")

/*-------Channel replies------*/
#define RPL_ENDOFWHO(nick, mask) (":" + SERVER_NAME + " 315 " + nick + " " + mask + " :End of WHO list\r\n")
#define RPL_LISTSTART(nick) (":" + SERVER_NAME + " 321 " + nick + " Channel :Users Name\r\n")
#define RPL_LIST(nick, channel, users, topic) (":" + SERVER_NAME + " 322 " + nick + " " + channel + " " + users + " :" + topic + "\r\n")
#define RPL_LISTEND(nick) (":" + SERVER_NAME + " 323 " + nick + " :End of /LIST\r\n")
#define RPL_CHANNELMODEIS(nick, channel, mode, params) (":" + SERVER_NAME + " 324 " + nick + " " + channel + " " + mode + " " + params + "\r\n")
#define RPL_NOTOPIC(nick, channel) (":" + SERVER_NAME + " 331 " + nick + " " + channel + " :No topic is set\r\n")
#define RPL_TOPIC(nick, channel, topic) (":" + SERVER_NAME + " 332 " + nick + " " + channel + " :" + topic + "\r\n")
#define RPL_TOPIC2(nickname, channel, topic) (": 332 " + nickname + " " + channel + " :" + topic + + "\r\n")
#define RPL_TOPICWHOTIME(nick, channel, setter, time) (":" + SERVER_NAME + " 333 " + nick + " " + channel + " " + setter + " " + time + "\r\n")
#define RPL_INVITING(nick, target, channel) (":" + SERVER_NAME + " 341 " + nick + " " + target + " " + channel + "\r\n")
#define RPL_WHOREPLY(nick, channel, user, host, server, nick2, flags, hops, realname) (":" + SERVER_NAME + " 352 " + nick + " " + channel + " " + user + " " + host + " " + server + " " + nick2 + " " + flags + " :" + hops + " " + realname + "\r\n")
#define RPL_NAMREPLY(nick, channel, users) (":" + SERVER_NAME + " 353 " + nick + " " + " " + channel + " :" + users + "\r\n")
#define RPL_ENDOFNAMES(nick, channel) (":" + SERVER_NAME + " 366 " + nick + " " + channel + " :End of /NAMES list\r\n")

/*-------MOTD Replies---------*/
#define RPL_MOTDSTART(nick) (":" + SERVER_NAME + " 375 " + nick + " :- " + SERVER_NAME + " Message of the day -\r\n")
#define RPL_MOTD(nick) (":" + SERVER_NAME + " 372 " + nick + " :-" + "\r\n")
#define RPL_ENDOFMOTD(nick) (":" + SERVER_NAME + " 376 " + nick + " :End of /MOTD command.\r\n")

/*---------Error replies-------*/
#define ERR_NOSUCHNICK(nick, target) (":" + SERVER_NAME + " 401 " + nick + " " + target + " :No such nick/channel\r\n")
#define ERR_NOSUCHCHANNEL(nick, channel) (":" + SERVER_NAME + " 403 " + nick + " " + channel + " :No such channel\r\n")
#define ERR_CANNOTSENDTOCHAN(nick, channel) (":" + SERVER_NAME + " 404 " + nick + " " + channel + " :Cannot send to channel\r\n")
#define ERR_TOOMANYCHANNELS(nick, channel) (":" + SERVER_NAME + " 405 " + nick + " " + channel + " :You have joined too many channels\r\n")
#define ERR_UNKNOWNCOMMAND(nick, command) (":" + SERVER_NAME + " 421 " + nick + " " + command + " :Unknown command\r\n")
#define ERR_NONICKNAMEGIVEN(nick) (":" + SERVER_NAME + " 431 " + nick + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(nick) (":" + SERVER_NAME + " 432 " + nick + " :Erroneous nickname\r\n")
#define ERR_NICKNAMEINUSE(nick) (":" + SERVER_NAME + " 433 * " + nick + " :Nickname is already in use\r\n")
#define ERR_NICKCOLLISION(nick) (":" + SERVER_NAME + " 436 " + nick + " :Nickname collision KILL\r\n")
#define ERR_USERNOTINCHANNEL(nick, target, channel) (":" + SERVER_NAME + " 441 " + nick + " " + target + " " + channel + " :They aren't on that channel\r\n")
#define ERR_NOTONCHANNEL(nick, channel) (":" + SERVER_NAME + " 442 " + nick + " " + channel + " :You're not on that channel\r\n")
#define ERR_USERONCHANNEL(nick, channel) (":" + SERVER_NAME + " 443 " + nick + " " + channel + " :is already on channel\r\n")
#define ERR_NEEDMOREPARAMS(nick, command) (":" + SERVER_NAME + " 461 " + nick + " " + command + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED(nick) (":" + SERVER_NAME + " 462 " + nick + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(nick) (":" + SERVER_NAME + " 464 " + nick + " :Password incorrect\r\n")
#define ERR_UNKNOWNMODE(nick, mode) (":" + SERVER_NAME + " 472 " + nick + " " + mode + " :is unknown mode char to me\r\n")
#define ERR_CHANNELISFULL(nick, channel) (":" + SERVER_NAME + " 471 " + nick + " " + channel + " :Cannot join channel (+l)\r\n")
#define ERR_INVITEONLYCHAN(nick, channel) (":" + SERVER_NAME + " 473 " + nick + " " + channel + " :Cannot join channel (+i)\r\n")
#define ERR_BANNEDFROMCHAN(nick, channel) (":" + SERVER_NAME + " 474 " + nick + " " + channel + " :Cannot join channel (+b)\r\n")
#define ERR_BADCHANMASK(nick, channel) (":" + SERVER_NAME + " 476 " + nick + " " + channel + " :Bad Channel Mask\r\n")
#define ERR_CHANOPRIVSNEEDED(nick, channel) (":" + SERVER_NAME + " 482 " + nick + " " + channel + " :You're not channel operator\r\n")
