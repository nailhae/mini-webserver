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
#define MAX_KEVENTS 100

void WebServer::closeClientSocket(UserData* udata, int fd)
{
	mChangeList.ChangeEvent(fd, EVFILT_READ, EV_DELETE, NULL);
	std::cout << Colors::BoldBlue << "close client:" << fd << Colors::Reset << std::endl;
	delete udata;
	close(fd);
}

void WebServer::ShutdownCgiPid(UserData* udata)
{
	int pid = udata->GetFd();
	std::cout << Colors::BoldBlue << "close cgi pid: " << pid << Colors::Reset << std::endl;
	// TODO child 살아있으면 kill
	kill(pid, SIGUSR1);
	delete udata;
	mChangeList.ChangeEvent(pid, EVFILT_TIMER, EV_DELETE, NULL);
}

void WebServer::closeCgiSocket(UserData* udata, int fd)
{
	std::cout << Colors::BoldBlue << "close cgi: " << fd << Colors::Reset << std::endl;
	close(fd);
	delete udata;
	mChangeList.ChangeEvent(fd, EVFILT_READ, EV_DELETE, NULL);
}

// static void setSocketKeepAlive(int fd, int cnt, int idle, int interval)
// {
// 	int on = true;

// 	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
// 	setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
// 	setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(idle));
// 	setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
// }

static void setSocketLinger(int fd)
{
	struct linger optVal;

	optVal.l_linger = 0;
	optVal.l_onoff = true;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, &optVal, sizeof(optVal));
	// setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optVal.l_onoff, sizeof(optVal));
}

int WebServer::acceptClientSocket(int fd, ServerBlock* serverPtr)
{
	int sock = 0;
	struct sockaddr_in adr;
	socklen_t adrSize;
	UserData* udata;

	adrSize = sizeof(adr);
	sock = accept(fd, (struct sockaddr*)&adr, &adrSize);
	if (sock == ERROR)
	{
		return (ERROR);
	}
	udata = new UserData(sock);
	udata->SetServerPtr(serverPtr);
	udata->SetSocketType(CLIENT_SOCKET);
	// setSocketKeepAlive(sock, 60, 5, 5);
	setSocketLinger(sock);
	fcntl(sock, F_SETFL, O_NONBLOCK);
	mChangeList.ChangeEvent(sock, EVFILT_READ, EV_ADD, udata);

	std::cout << Colors::Blue << "Connected Client: " << sock << Colors::Reset << std::endl;
	return (0);
}

void WebServer::WaitForClientConnection(void)
{
	int kq;

	if (InitServer() == ERROR)
		exit(EXIT_FAILURE);
	kq = kqueue();
	if (kq == -1)
	{
		Error::Print("Open Error");
		exit(EXIT_FAILURE);
	}

	struct kevent eventList[MAX_KEVENTS];
	int occurEventNum;
	int readLen;

	while (1)
	{
		mChangeList.PrintEveryList();
		occurEventNum =
			kevent(kq, &mChangeList.GetKeventVector()[0], mChangeList.GetSize(), eventList, MAX_KEVENTS, NULL);
		if (occurEventNum == ERROR)
		{
			Error::Print("kevent() error");
			exit(EXIT_FAILURE);
		}
		mChangeList.ClearEvent();
		for (int i = 0; i < occurEventNum; i++)
		{
			std::cout << "ident: " << eventList[i].ident << std::endl;
			std::cout << "filter: " << eventList[i].filter << std::endl;
			std::cout << "flags: " << eventList[i].flags << std::endl;
			std::cout << "fflags: " << eventList[i].fflags << std::endl;
			std::cout << "data: " << eventList[i].data << std::endl;

			if ((eventList[i].flags & EV_ERROR) == EV_ERROR)
			{
				std::cout << "data contents errno: " << errno << std::endl;
				close(eventList[i].ident);
				exit(1);
			}
			else if ((eventList[i].flags & EV_EOF) == EV_EOF)
			{
				std::cout << Colors::BoldRed << "망할 왜 eof야" << std::endl;
				close(eventList[i].ident);
				if (eventList[i].udata != NULL)
				{
					delete static_cast<UserData*>(eventList[i].udata);
				}
				continue;
			}
			UserData* currentUdata = static_cast<UserData*>(eventList[i].udata);
			if (currentUdata->GetSocketType() == SERVER_SOCKET)
			{
				if (acceptClientSocket(eventList[i].ident, currentUdata->GetServerPtr()) == ERROR)
					continue;
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
				// CGI 처리.
				if (eventList[i].filter == EVFILT_WRITE)
				{
					if (currentUdata->SendToCgi() == ERROR)
					{
						closeCgiSocket(currentUdata, eventList[i].ident);
						std::cout << "force close cgi: " << eventList[i].ident << std::endl;
					}
					// 소켓통신에서 write
				}
				if (eventList[i].filter == EVFILT_READ)
				{
					// read
					readLen = currentUdata->RecvFromCgi();
					if (readLen == 0)
					{
						// waitpid -> 0인지 확인
						int status;
						waitpid(currentUdata->GetPid(), &status, WNOHANG);
						if (WIFEXITED(status) == true)
						{
							std::cout << "exit code: " << WEXITSTATUS(status) << std::endl;
							if (WEXITSTATUS(status) == 0)
							{
								// 200 페이지 작성
								currentUdata->GeneratePostResponse(200);
							}
							else
							{
								// 502 페이지 작성
								currentUdata->GeneratePostResponse(502);
							}
						}
						else if (WIFSIGNALED(status) == true)
						{
							// 504 에러 페이지 작성
							std::cout << "exit signal: " << status << std::endl;
							currentUdata->GeneratePostResponse(504);
						}
						else
						{
							std::cout << "exit signal: " << status << std::endl;
							// 500 에러 페이지 작성
							currentUdata->GeneratePostResponse(500);
						}

						ChangeEvent(currentUdata->GetClientUdata()->GetFd(), EVFILT_WRITE, EV_ADD,
									currentUdata->GetClientUdata());
						closeCgiSocket(currentUdata, eventList[i].ident);
					}
					else if (readLen < 0)
					{
						closeCgiSocket(currentUdata, eventList[i].ident);
						std::cout << "force close client: " << eventList[i].ident << std::endl;
					}
					else
						currentUdata->RecvFromCgi();
				}
			}
			else if (eventList[i].filter == EVFILT_TIMER)
			{
				// kill child process
				// 서버 에러.
				ShutdownCgiPid(currentUdata);
			}
		}
		memset(eventList, 0, sizeof(eventList));
	}
}
