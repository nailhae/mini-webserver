#include "ChangeList.hpp"
#include "Colors.hpp"
#include "UserData.hpp"
#include "WebServer.hpp"
#define ERROR -1

static void setServerSocketOption(int fd)
{
	int optVal = true;

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal));
}

static int createSocket(int port)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;

	if (serverSocket == -1)
	{
		std::cout << "Error: socket()" << std::endl;
		return ERROR;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);
	setServerSocketOption(serverSocket);
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cout << "Error: bind() error" << std::endl;
		close(serverSocket);
		return -1;
	}
	if (listen(serverSocket, SOMAXCONN) == -1)
	{
		std::cout << "Error: listen() error" << std::endl;
		close(serverSocket);
		return -1;
	}
	return serverSocket;
}

int WebServer::InitServer(void)
{
	int serverSocket;
	UserData* udata;

	for (std::vector<ServerBlock*>::iterator it = mHttp->serverList.begin(); it != mHttp->serverList.end(); it++)
	{
		serverSocket = createSocket((*it)->listenPort);
		if (serverSocket == ERROR)
		{
			std::cerr << "Error: server socket" << std::endl;
			return (ERROR);
		}
		udata = new UserData(serverSocket);
		udata->SetServerPtr(*it);
		udata->SetSocketType(SERVER_SOCKET);
		mChangeList.ChangeEvent(serverSocket, EVFILT_READ, EV_ADD, udata);
		std::cout << Colors::BoldGreen << udata->GetServerPtr()->serverName << ": Socket " << serverSocket
				  << " is now listen" << std::endl;
	}
	return (0);
}
