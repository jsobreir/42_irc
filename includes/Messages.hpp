#pragma once

#include <string>

// Example global constants (define these somewhere in your server code)
extern std::string _serverName_g;
extern std::string _creationDate_g;

/*------Connection replies-----*/
inline std::string RPL_WELCOME(const std::string& nick) {
    return "001 " + nick + " :Welcome to " + _serverName_g + ", " + nick + "\r\n";
}

inline std::string RPL_YOURHOST() {
    return "002 :Your host is " + _serverName_g + ", running version 1.0\r\n";
}

inline std::string RPL_CREATED() {
    return "003 :This server was created " + _creationDate_g + "\r\n";
}

inline std::string RPL_MYINFO(const std::string& nick, const std::string& version) {
    return "004 " + nick + " " + _serverName_g + " " + version + " :Available user modes: io, channel modes: tkl\r\n";
}

inline std::string RPL_JOIN(const std::string& nick, const std::string& user, const std::string& host, const std::string& channel) {
    return ":" + nick + "!" + user + "@" + host + " JOIN " + channel + "\r\n";
}

/*-------Channel replies------*/
inline std::string RPL_ENDOFWHO(const std::string& nick, const std::string& mask) {
    return ":315 " + nick + " " + mask + " :End of WHO list\r\n";
}

inline std::string RPL_LISTSTART(const std::string& channel, const std::string& name) {
    return "321 " + channel + " Users " + name + "\r\n";
}

inline std::string RPL_LIST(const std::string& content) {
    return "322 " + content + "\r\n";
}

inline std::string RPL_LISTEND() {
    return "323 :End of /LIST\r\n";
}

inline std::string RPL_CHANNELMODEIS(const std::string& channel, const std::string& mode, const std::string& params) {
    return "324 " + channel + " " + mode + " " + params + "\r\n";
}

inline std::string RPL_NOTOPIC(const std::string& channel) {
    return "331 " + channel + " :No topic is set\r\n";
}

inline std::string RPL_TOPIC(const std::string& channel, const std::string& topic) {
    return "332 " + channel + " :" + topic + "\r\n";
}

inline std::string RPL_TOPIC2(const std::string& nickname, const std::string& channel, const std::string& topic) {
    return "332 " + nickname + " " + channel + " :" + topic + "\r\n";
}

inline std::string RPL_TOPICWHOTIME(const std::string& nick, const std::string& channelName, const std::string& time) {
    return ":" + _serverName_g + " 333 " + nick + " " + channelName + " " + time + "\r\n";
}

inline std::string RPL_INVITING(const std::string& nick, const std::string& channel) {
    return "341 " + nick + " " + channel + "\r\n";
}

inline std::string RPL_WHOREPLY(const std::string& nickname, const std::string& host, const std::string& channelname, const std::string& user, const std::string& realname, const std::string& flag) {
    return ":" + host + " 352 " + nickname + " " + channelname + " " + host + " " + _serverName_g + " " + user + " " + flag + " :2 " + realname + "\r\n";
}

inline std::string RPL_NAMREPLY(const std::string& nick, const std::string& channel, const std::string& users) {
    return ":" + _serverName_g + " 353 " + nick + " = " + channel + " :" + users + "\r\n";
}

inline std::string RPL_ENDOFNAMES(const std::string& channelName) {
    return "366 " + channelName + " :End of /NAMES list\r\n";
}

/*-------Miscellaneous---------*/
inline std::string RPL_MOTDSTART(const std::string& nick) {
    return "375 " + nick + " - " + _serverName_g + " Message of the day - \r\n";
}

inline std::string RPL_MOTD(const std::string& nick) {
    return "372 " + nick + "  Welcome to " + _serverName_g + ", and remember what happens in " + _serverName_g + " stays in " + _serverName_g + " \xF0\x9F\x98\x8E.\r\n";
}

inline std::string RPL_ENDOFMOTD(const std::string& nick) {
    return "376 " + nick + " :End of /MOTD command.\r\n";
}

/*---------Error replies-------*/
inline std::string ERR_NOSUCHNICK(const std::string& nick) {
    return "401 " + nick + " :No such nick/channel\r\n";
}

inline std::string ERR_NOSUCHCHANNEL(const std::string& channel) {
    return "403 " + channel + " :No such channel\r\n";
}

inline std::string ERR_CANNOTSENDTOCHAN(const std::string& nick, const std::string& channel) {
    return "404 " + nick + " " + channel + " :Cannot send to channel\r\n";
}

inline std::string ERR_TOOMANYCHANNELS(const std::string& nick, const std::string& channel) {
    return "405 " + nick + " " + channel + " :You have joined too many channels\r\n";
}

inline std::string ERR_UNKNOWNCOMMAND(const std::string& command) {
    return "421 " + command + " :Unknown command\r\n";
}

inline std::string ERR_NONICKNAMEGIVEN() {
    return "431 :No nickname given\r\n";
}

inline std::string ERR_NICKNAMEINUSE(const std::string& nick) {
    return "433 * " + nick + " :Nickname is already in use\r\n";
}

inline std::string ERR_ERRONEUSNICKNAME(const std::string& nick) {
    return "432 " + nick + " :Erroneous nickname\r\n";
}

inline std::string ERR_NICKCOLLISION(const std::string& nick) {
    return "436 " + nick + " :Nickname collision KILL\r\n";
}

inline std::string ERR_NOTONCHANNEL(const std::string& nick, const std::string& channelName) {
    return "442 " + nick + " " + channelName + " :You're not on that channel\r\n";
}

inline std::string ERR_USERONCHANNEL(const std::string& user, const std::string& nick, const std::string& channel) {
    return "443 " + nick + " " + user + " " + channel + " :is already on channel\r\n";
}

inline std::string ERR_NEEDMOREPARAMS(const std::string& command) {
    return "461 " + command + " :Not enough parameters\r\n";
}

inline std::string ERR_ALREADYREGISTRED() {
    return "462 :You may not reregister\r\n";
}

inline std::string ERR_PASSWDMISMATCH() {
    return "464 :Password incorrect\r\n";
}

inline std::string ERR_CHANNELISFULL(const std::string& nick, const std::string& channelName) {
    return "471 " + nick + " " + channelName + " :Cannot join channel (+l)\r\n";
}

inline std::string ERR_UNKNOWNMODE(const std::string& mode) {
    return "472 " + mode + " :is unknown mode char to me\r\n";
}

#define ERR_INVITEONLYCHAN(nick, channelName) ("473 " + nick + channelName + " :Cannot join channel (+i)\n")

//#define ERR_BANNEDFROMCHAN(nick, channelName) ("474 " + nick + channelName + " :Cannot join channel (+b)\n")

inline std::string ERR_BADCHANMASK(const std::string& channelName) {
    return "476 " + channelName + " :Invalid channel name\r\n";
}

inline std::string ERR_CHANOPRIVSNEEDED(const std::string& client, const std::string& channel) {
    return ":" + _serverName_g + " 482 " + client + " " + channel + " :You're not channel operator\r\n";
}
