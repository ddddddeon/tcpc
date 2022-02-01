#include <iostream>

#include "server.h"

using namespace std;

#define PORT 9000
#define HOST "localhost"

int main(int argc, char *argv[])
{
	Logger logger;

	int port = PORT;
	if (argc > 1)
	{
		int portArg = std::atoi(argv[1]);
		if (portArg > 0 && portArg < 65536)
		{
			port = portArg;
		}
	}

	Server server(HOST, port, logger);

	try
	{
		server.Start();
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}

	return 0;
}
