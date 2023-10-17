#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sstream>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "ChangeList.hpp"
#include "Colors.hpp"
#include "Error.hpp"
#include "WebServer.hpp"

#define BUFFER_SIZE 1024
#define MAX_KEVENTS 10
#define ERROR -1
#define CRLF "\r\n"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		Error::Print("no conf file");
		exit(1);
	}

	WebServer* webServer;
	std::string confFile = argv[1];

	webServer = WebServer::GetInstance(confFile);
	webServer->WaitForClientConnection();
	WebServer::DeleteInstance();

	return (0);
}