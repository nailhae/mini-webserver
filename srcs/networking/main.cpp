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
#include <unistd.h>
#include <vector>
#include "ChangeList.hpp"
#include "Colors.hpp"
#include "Error.hpp"

#define BUFFER_SIZE 1024
#define MAX_KEVENTS 10
#define ERROR -1
#define CRLF "\r\n"

typedef struct clientAddr
{
	int sock;
	struct sockaddr_in adr;
	socklen_t adr_size;
} clientAddr;

void acceptClientSocket(clientAddr& client, ChangeList& changeList)
{
	// 서버 구조체..?가 가지고 있는 것이 나을 것인지?
	client.adr_size = sizeof(client.adr);
	client.sock = accept(서버소켓.udata.fd, reinterpret_cast<struct sockaddr*>(&client.adr), &client.adr_size);
	changeList.changeEvent(client.sock, EVFILT_READ, EV_ADD);
	fcntl(client.sock, F_SETFL, O_NONBLOCK);
	std::cout << Colors::Blue << "Connected Client: " << client.sock << Colors::Reset << std::endl;
}

void closeClientSocket(UserData& udata, ChangeList& changeList)
{
	std::cout << Colors::BoldGreen << "close client:" << udata.GetFd() << Colors::Reset << std::endl;
	delete &udata;
	changeList.changeEvent(udata.GetFd(), EVFILT_READ, EV_DELETE);
	close(udata.GetFd());
}

int main()
{
	// parsing
	// init Server + socket listen

	int kq = kqueue();
	if (kq == -1)
	{
		Error::Print("Open Error");
		return (1);
	}

	ChangeList changeList;
	struct kevent eventList[MAX_KEVENTS];
	int occurEventNum;

	/**
	 * @brief 서버 Listen 소켓 옵션 설정
	 * int optVal = true;
	 * while (서버 listen port 개수)
	 * {
	 * 	changeList(서버 listen 소켓, EVFILT_READ, EV_ADD);
	 * 	setsockopt(서버 listen 소켓, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
	 * 	setsockopt(서버 listen 소켓, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal));
	 * }
	 */

	/**
	 * @brief HTTP/1.1 default 값 keep alive 옵션 + 우아한 소켓 종료를 위한 linger 옵션 적용
	 * struct linger lingerVal;
	 * lingerVal.l_onoff = true;
	 */

	clientAddr client;
	int len;

	while (1)
	{
		occurEventNum =
			kevent(kq, changeList.getKeventVector().data(), changeList.getSize(), eventList, MAX_KEVENTS, NULL);
		if (occurEventNum == ERROR)
		{
			Error::Print("kevent() error");
			exit(EXIT_FAILURE); // exit으로 부수고 나가기
		}
		changeList.clearEvent();
		for (int i = 0; i < occurEventNum; i++)
		{
			UserData* currentUdata = static_cast<UserData*>(eventList[i].udata);
			if (eventList[i].ident == 서버소켓.udata.fd)
				acceptClientSocket(client, changeList);
			else if (eventList[i].filter & EVFILT_READ)
			{
				len = currentUdata->RecvFromClient(eventList[i].ident);
				if (len == 0 ||
					(eventList[i].fflags & EV_EOF) == true) // close request -> write 이벤트도 등록해야 한다.
					closeClientSocket(*currentUdata, changeList);
				else if (len < 0 || (eventList[i].fflags & EV_ERROR) == true)
				{
					close(eventList[i].ident);
					Error::Print("recv error: force close client: " + eventList[i].ident);
				}
				else
					currentUdata->GenerateResponse();
			}
		}
	}
	//	close(각 서버 소켓);
	close(kq);
	return (0);
}