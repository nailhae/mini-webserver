#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include "Error.hpp"

int createSocket(int port)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		Error::Print("socket()");
		return -1;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		Error::Print("bind() error");
		close(serverSocket);
		return -1;
	}
	if (listen(serverSocket, 10) == -1) // back log 일반적으로 10 -100
	{
		Error::Print("listen() error");
		close(serverSocket);
		return -1;
	}
	return serverSocket;
}

int initializeServer(const struct httpBlock& httpConfig)
{
	for (std::vector<serverBlock>::iterator it = httpConfig.server_list.begin(); it != httpConfig.server_list.end();
		 it++)
	{
		int serverSocket = createSocket(it->listen_port);
		if (serverSocket == -1)
		{
			Error::Print("server socket");
			return -1;
		}
	}
	return 0;
}
