#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "ChangeList.hpp"
#include "Colors.hpp"
#include "Error.hpp"
#include "UserData.hpp"
#include "WebServer.hpp"
#include "dataSet.hpp"
#define ERROR -1
#define MAX_KEVENTS 10

void WebServer::closeClientSocket(UserData* udata, int fd)
{
	std::cout << Colors::BoldBlue << "close client:" << fd << Colors::Reset << std::endl;
	mChangeList.ChangeEvent(fd, EVFILT_READ, EV_DELETE, NULL);
	mChangeList.ChangeEvent(fd, EVFILT_WRITE, EV_DELETE, NULL);
	if (udata->GetClientUdata() != NULL)
	{
		closeCgiSocket(udata->GetClientUdata(), udata->GetClientUdata()->GetFd());
	}
	delete udata;
	close(fd);
}

void WebServer::ShutdownCgiPid(UserData* udata)
{
	int pid = udata->GetPid();

	std::cout << Colors::BoldBlue << "timer 발생 close cgi pid: " << pid << Colors::Reset << std::endl;
	kill(pid, SIGTERM);
	waitpid(pid, NULL, 0);
	udata->GeneratePostResponse(504);

	mChangeList.ChangeEvent(udata->GetFd(), EVFILT_READ, EV_DISABLE | EV_DELETE, udata);
	mChangeList.ChangeEvent(udata->GetFd(), EVFILT_WRITE, EV_DISABLE | EV_DELETE, udata);

	mChangeList.ChangeEvent(udata->GetClientUdata()->GetFd(), EVFILT_READ, EV_DISABLE, udata->GetClientUdata());
	mChangeList.ChangeEvent(udata->GetClientUdata()->GetFd(), EVFILT_WRITE, EV_ENABLE, udata->GetClientUdata());
	delete udata;
}

void WebServer::closeCgiSocket(UserData* udata, int fd)
{
	std::cout << Colors::BoldBlue << "close cgi: " << fd << Colors::Reset << std::endl;
	mChangeList.ChangeEvent(fd, EVFILT_READ, EV_DELETE, NULL);
	mChangeList.ChangeEvent(fd, EVFILT_WRITE, EV_DELETE, NULL);
	mChangeList.ChangeEvent(udata->GetPid(), EVFILT_TIMER, EV_DELETE, NULL);
	kill(udata->GetPid(), SIGTERM);
	waitpid(udata->GetPid(), NULL, 0);
	udata->SetClientUdataNULL();
	delete udata;
	close(fd);
}

static void setSocketLinger(int fd)
{
	struct linger optVal;

	optVal.l_linger = 0;
	optVal.l_onoff = true;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, &optVal, sizeof(optVal));
}

void WebServer::acceptClientSocket(int fd, ServerBlock* serverPtr)
{
	int sock;
	struct sockaddr_in adr;
	socklen_t adrSize;
	UserData* udata;

	adrSize = sizeof(adr);
	sock = accept(fd, (struct sockaddr*)&adr, &adrSize);
	if (sock == -1)
	{
		return;
	}
	udata = new UserData(sock);
	udata->SetServerPtr(serverPtr);
	udata->SetSocketType(CLIENT_SOCKET);
	setSocketLinger(sock);
	mChangeList.ChangeEvent(sock, EVFILT_READ, EV_ADD | EV_ENABLE, udata);
	mChangeList.ChangeEvent(sock, EVFILT_WRITE, EV_ADD | EV_DISABLE, udata);

	fcntl(sock, F_SETFL, O_NONBLOCK);
	std::cout << Colors::Blue << "Connected Client: " << sock << Colors::Reset << std::endl;
}

void WebServer::InitKq(void)
{
	mKq = kqueue();
	if (mKq == -1)
	{
		Error::Print("Open Error");
		exit(EXIT_FAILURE);
	}
	mChangeList.setKq(mKq);
}

void WebServer::HandlingServerSocket(int serverSocket, ServerBlock* serverPtr)
{
	acceptClientSocket(serverSocket, serverPtr);
}

void WebServer::HandlingClientSocket(struct kevent& event, UserData* udata)
{
	int readLen = 0;

	if (event.filter == EVFILT_READ)
	{
		readLen = udata->RecvFromClient();
		if (readLen == 0 || (event.flags & EV_EOF))
		{
			closeClientSocket(udata, event.ident);
		}
		else if (readLen < 0)
		{
			closeClientSocket(udata, event.ident);
			std::cout << "force close client: " << event.ident << std::endl;
		}
		else
			udata->ReadRequest();
	}
	else if (event.filter == EVFILT_WRITE)
	{
		if (udata->SendToClient(event.ident) == ERROR)
		{
			closeClientSocket(udata, event.ident);
			std::cout << "force close client: " << event.ident << std::endl;
		}
	}
}

void WebServer::HandlingCGISocket(struct kevent& event, UserData* udata)
{
	int readLen = 0;

	if (event.filter == EVFILT_WRITE)
	{
		if (udata->SendToCgi() == ERROR)
		{
			closeCgiSocket(udata, event.ident);
			std::cout << "force close cgi: " << event.ident << std::endl;
		}
	}
	if (event.filter == EVFILT_READ)
	{
		readLen = udata->RecvFromCgi();
		std::cout << "readLen : " << readLen << "\n";
		if (readLen == 0)
		{
			int status = 0;

			kill(udata->GetPid(), SIGTERM);
			waitpid(udata->GetPid(), &status, 0);
			if (WIFEXITED(status) == true)
			{
				std::cout << "exit code: " << WEXITSTATUS(status) << std::endl;
				if (WEXITSTATUS(status) == 0)
				{
					udata->GeneratePostResponse(200);
				}
				else
				{
					udata->GeneratePostResponse(502);
				}
			}
			else
			{
				std::cout << "signal: " << WTERMSIG(status) << std::endl;
				udata->GeneratePostResponse(502);
			}
			ChangeEvent(udata->GetClientUdata()->GetFd(), EVFILT_WRITE, EV_ENABLE, udata->GetClientUdata());
			closeCgiSocket(udata, event.ident);
		}
		else if (readLen < 0)
		{
			closeCgiSocket(udata, event.ident);
			std::cout << "force close cgi: " << event.ident << std::endl;
		}
	}
}

void WebServer::HandlingTimer(UserData* udata)
{
	ShutdownCgiPid(udata);
}

void WebServer::WaitForClientConnection(void)
{
	struct kevent eventList[MAX_KEVENTS];
	int occurEventNum;

	InitKq();
	if (InitServer() == ERROR)
		exit(EXIT_FAILURE);
	while (1)
	{
		occurEventNum =
			kevent(mKq, mChangeList.GetKeventVector().data(), mChangeList.GetSize(), eventList, MAX_KEVENTS, NULL);
		if (occurEventNum == ERROR)
		{
			Error::Print("kevent() error");
			perror("kevent");
			exit(EXIT_FAILURE);
		}
		mChangeList.ClearEvent();
		for (int i = 0; i < occurEventNum; i++)
		{
			UserData* currentUdata = static_cast<UserData*>(eventList[i].udata);
			if (eventList[i].flags & EV_ERROR)
			{
				std::cout << "EV_ERROR: " << eventList[i].data << std::endl;
				continue;
			}
			if (eventList[i].filter == EVFILT_TIMER)
			{
				ShutdownCgiPid(currentUdata);
			}
			else if (currentUdata->GetSocketType() == SERVER_SOCKET)
			{
				HandlingServerSocket(eventList[i].ident, currentUdata->GetServerPtr());
			}
			else if (currentUdata->GetSocketType() == CLIENT_SOCKET)
			{
				HandlingClientSocket(eventList[i], currentUdata);
			}
			else if (currentUdata->GetSocketType() == CGI_SOCKET)
			{
				HandlingCGISocket(eventList[i], currentUdata);
			}
		}
	}
}
