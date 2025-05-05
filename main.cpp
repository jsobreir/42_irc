#include "irc.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <server> <port>" << std::endl;
		return 1;
	}
}