#include <iostream>
#include <signal.h>
#include <asio.hpp>

#include "main.h"
#include "server.h"

asio::ip::address_v4 INTERFACE = asio::ip::address_v4::loopback();
int PORT = 9000;

int main(int argc, char *argv[])
{
	parse_args(argc, argv);

	Logger logger;
	Server server(INTERFACE, PORT, logger);

	try
	{
		server.Start();
	}
	catch (std::exception &e)
	{
		logger.Error(e.what());
	}

	return 0;
}

void parse_args(int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, ":lw:i:p:")) != -1)
	{
		switch (opt)
		{
		case 'l':
			INTERFACE = asio::ip::address_v4::loopback();
			break;
		case 'a':
			INTERFACE = asio::ip::address_v4::any();
			break;
		case 'i':
			INTERFACE = asio::ip::make_address_v4(optarg);
			break;
		case 'p':
			std::cout << optarg << std::endl;
			PORT = atoi(optarg);
			break;
		}
	}
}
