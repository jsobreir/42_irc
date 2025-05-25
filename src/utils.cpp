#include "IRC.hpp"

// Parse a single IRC line into command + params
IRCCommand parseIRCLine(const std::string &line) {
    IRCCommand result;
    std::istringstream ss(line);
    ss >> result.command;
    std::string token;

    // Collect all params except last trailing param
    while (ss >> token) {
        if (token[0] == ':') {
            // Trailing param - consume rest of line as one param (without ':')
            std::string trailing = token.substr(1);
            std::string rest;
            std::getline(ss, rest);
            trailing += rest;
            // Trim trailing spaces if needed
            result.args.push_back(trailing);
            break;
        }
        result.args.push_back(token);
    }
    return result;
}