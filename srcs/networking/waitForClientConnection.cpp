#include <netinet/in.h>
#include <netinet/tcp.h>
#include "ChangeList.hpp"
#include "UserData.hpp"
#include "WebServer.hpp"
#define ERROR -1
#define MAX_KEVENTS 10

void WebServer::closeClientSocket(UserData* udata)
{
	std::cout << Colors::BoldGreen << "close client:" << udata.GetFd() << Colors::Reset << std::endl;
	delete udata;
	mChangeList.ChangeEvent(udata->GetFd(), EVFILT_READ, EV_DELETE, NULL);
	close(udata->GetFd());
}

static void setSocketKeepAlive(int fd, int cnt, int idle, int interval)
{
	int on = true;

	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(idle));
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
}

static void setSocketLinger(int fd)
{
	struct linger optVal;

	optVal.l_linger = true;
	optVal.l_onoff = true;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, optVal, sizeof(optVal));
}

static void WebServer::acceptClientSocket(int fd, ServerBlock* serverPtr)
{
	int sock;
	struct sockaddr_in adr;
	socklen_t adrSize;
	UserData* udata;
	struct linger lingerVal;
	lingerVal.l_onoff = true;

	adrSize = sizeof(adr);
	sock = accept(fd, (struct sockaddr *)&adr, &adrSize);
	udata->SetServerPtr(serverPtr);
	udata->SetSocketType(CLIENT_SOCKET);
	setSocketKeepAlive(sock, 60, 5, 5);
	setSocketLinger(sock);
	mChangeList.ChangeEvent(sock, EVFILT_READ, EV_ADD, udata);
	fcntl(sock, F_SETFL, O_NONBLOCK);
	std::cout << Colors::Blue << "Connected Client: " << sock << Colors::Reset << std::endl;
}

void WebServer::WaitForClientConnection(void)
{
	int kq;

	if (InitServer() == ERROR)
		exit(EXIT_FAILURE);
	kq = kqueue();
	if (kq == -1)
	{
		ERROR::Print("Open Error");
		exit(EXIT_FAILURE);
	}

	struct kevent eventList[MAX_KEVENTS];
	int occurEventNum;
	int readLen;

	while (1)
	{
		occurEventNum =
			kevent(kq, mChangeList.GetKeventVector().data(), mChangeList.GetSize(), eventList, MAX_KEVENTS, NULL);
		if (occurEventNum == ERROR)
		{
			Error::Print("kevent() error");
			exit(EXIT_FAILURE);
		}
		changeList.ClearEvent();
		for (int i = 0; i < occurEventNum; i++)
		{
			UserData* currentUdata = static_cast<UserData*>(eventList[i].udata);
			if (currentUdata.GetSocketType() == SERVER_SOCKET)
			{
				acceptClientSocket(eventList[i].ident, currentUdata);
			}
			else if (currentUdata.GetSocketType() == CLIENT_SOCKET)
			{
				readLen = currentUdata->RecvFromClient();
				if (readLen == 0)
				{
					closeClientSocket(currentUdata);
				}
				else if (len < 0)
				{
					close(eventList[i].ident);
					Error::Print("recv error: force close client: " + eventList[i].ident);
				}
				else
				{
					// AMethod 완성 되면 이걸로 돌려보기
					// currentUdata->GetMethod().GenerateResponse();
					// * 임시 *
					currentUdata->GenerateResponse();
				}
			}
		}
	}
}
