#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>

int createSocket(int port)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cout << "Error: socket()" << std::endl;
		return -1;
	}

	struct sockaddr_in	serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cout << "Error: bind() error" << std::endl;
		close(serverSocket);
		return -1;
	}
	if (listen(serverSocket, 10) == -1) //back log 일반적으로 10 -100
	{
		std::cout << "Error: listen() error" << std::endl;
		close(serverSocket);
		return -1;
	}
	return serverSocket;
}

int	initializeServer(const struct httpBlock& httpConfig)
{
	for (std::vector<serverBlock>::iterator it = httpConfig.server_list.begin(); it != httpConfig.server_list.end(); it++)
	{
		int serverSocket = createSocket(it->listen_port);
		if(serverSocket == -1)
		{
			std::cout << "Error: server socket" << std::endl;
			return -1;
		}
	}
	return 0;
}
