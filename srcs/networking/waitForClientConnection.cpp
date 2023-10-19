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
	int status;

	std::cout << Colors::BoldBlue << "close cgi pid: " << pid << Colors::Reset << std::endl;
	if (waitpid(pid, &status, WNOHANG) == 0)
		kill(pid, SIGUSR1);
	udata->GeneratePostResponse(504);
	mChangeList.ChangeEvent(udata->GetClientUdata()->GetFd(), EVFILT_WRITE, EV_ENABLE, udata->GetClientUdata());
	std::cout << "udata fd write 킴: " << udata->GetClientUdata()->GetFd() << std::endl;
	delete udata;
}

void WebServer::closeCgiSocket(UserData* udata, int fd)
{
	std::cout << Colors::BoldBlue << "close cgi: " << fd << Colors::Reset << std::endl;
	mChangeList.ChangeEvent(fd, EVFILT_READ, EV_DELETE, NULL);
	mChangeList.ChangeEvent(fd, EVFILT_WRITE, EV_DELETE, NULL);
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
	udata = new UserData(sock);
	udata->SetServerPtr(serverPtr);
	udata->SetSocketType(CLIENT_SOCKET);
	setSocketLinger(sock);
	mChangeList.ChangeEvent(sock, EVFILT_READ, EV_ADD | EV_ENABLE, udata);
	mChangeList.ChangeEvent(sock, EVFILT_WRITE, EV_ADD | EV_DISABLE, udata);

	fcntl(sock, F_SETFL, O_NONBLOCK);
	std::cout << Colors::Blue << "Connected Client: " << sock << Colors::Reset << std::endl;
}

void WebServer::WaitForClientConnection(void)
{
	int kq;
	struct kevent eventList[MAX_KEVENTS];
	int occurEventNum;
	int readLen;

	if (InitServer() == ERROR)
		exit(EXIT_FAILURE);
	kq = kqueue();
	if (kq == -1)
	{
		Error::Print("Open Error");
		exit(EXIT_FAILURE);
	}
	mChangeList.setKq(kq);

	while (1)
	{
		occurEventNum =
			kevent(kq, mChangeList.GetKeventVector().data(), mChangeList.GetSize(), eventList, MAX_KEVENTS, NULL);
		if (occurEventNum == ERROR)
		{
			Error::Print("kevent() error");
			exit(EXIT_FAILURE);
		}
		mChangeList.ClearEvent();
		for (int i = 0; i < occurEventNum; i++)
		{
			UserData* currentUdata = static_cast<UserData*>(eventList[i].udata);
			if ((eventList[i].flags & EV_ERROR) == EV_ERROR)
			{
				close(eventList[i].ident);
				continue;
			}
			if (eventList[i].filter == EVFILT_TIMER)
			{
				ShutdownCgiPid(currentUdata);
			}
			if (currentUdata->GetSocketType() == SERVER_SOCKET)
			{
				acceptClientSocket(eventList[i].ident, currentUdata->GetServerPtr());
			}
			else if (currentUdata->GetSocketType() == CLIENT_SOCKET)
			{
				if (eventList[i].filter == EVFILT_READ)
				{
					readLen = currentUdata->RecvFromClient();
					if (readLen == 0)
					{
						closeClientSocket(currentUdata, eventList[i].ident);
					}
					else if (readLen < 0)
					{
						closeClientSocket(currentUdata, eventList[i].ident);
						std::cout << "force close client: " << eventList[i].ident << std::endl;
					}
					else
						currentUdata->ReadRequest();
				}
				else if (eventList[i].filter == EVFILT_WRITE)
				{
					if (currentUdata->SendToClient(eventList[i].ident) == ERROR)
					{
						closeClientSocket(currentUdata, eventList[i].ident);
						std::cout << "force close client: " << eventList[i].ident << std::endl;
					}
				}
			}
			else if (currentUdata->GetSocketType() == CGI_SOCKET)
			{
				if (eventList[i].filter == EVFILT_WRITE)
				{
					if (currentUdata->SendToCgi() == ERROR)
					{
						closeCgiSocket(currentUdata, eventList[i].ident);
						std::cout << "force close cgi: " << eventList[i].ident << std::endl;
					}
				}
				if (eventList[i].filter == EVFILT_READ)
				{
					readLen = currentUdata->RecvFromCgi();
					std::cout << "redLen : " << readLen << "\n";
					if (readLen == 0)
					{
						// currentUdata->GeneratePostResponse(200);
						int status = 0;
						if (waitpid(currentUdata->GetPid(), &status, WNOHANG) != 0)
						{
							if (WIFEXITED(status) == true)
							{
								std::cout << "exit code: " << WEXITSTATUS(status) << std::endl;
								if (WEXITSTATUS(status) == 0)
								{
									currentUdata->GeneratePostResponse(200);
								}
								else
								{
									currentUdata->GeneratePostResponse(502);
								}
							}
							else if (WIFSIGNALED(status) == true && WTERMSIG(status) == SIGUSR1)
							{
								std::cout << "exit signal: " << WTERMSIG(status) << std::endl;
								currentUdata->GeneratePostResponse(504);
							}
							else
							{
								std::cout << "exit signal: " << WTERMSIG(status) << std::endl;
								currentUdata->GeneratePostResponse(500);
							}
						}
						ChangeEvent(currentUdata->GetClientUdata()->GetFd(), EVFILT_WRITE, EV_ENABLE,
									currentUdata->GetClientUdata());
						closeCgiSocket(currentUdata, eventList[i].ident);
					}
					else if (readLen < 0)
					{
						closeCgiSocket(currentUdata, eventList[i].ident);
						std::cout << "force close client: " << eventList[i].ident << std::endl;
					}
				}
			}
		}
	}
}
