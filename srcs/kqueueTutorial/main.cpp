#include <cerrno> // 추가: errno를 사용하기 위한 헤더 파일
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#define NUM_REQUEST 20
#define CGI 0
#define WEBSERV 1

#define MIN_TIME 50
#define ADD_TIME 0

#define TIME_LIMIT_MS 3000

// 파일 디스크립터를 Non-Blocking 모드로 설정하는 함수
int setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl");
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

void changeEvents(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags,
				  uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	changeList.push_back(temp_event);
}

int main()
{
	std::cout << "NUM_REQUEST: " << NUM_REQUEST << std::endl;

	int sockets[NUM_REQUEST][2];
	const int bufferSize = 1024;
	char buffer[bufferSize];

	int kq = kqueue();
	if (kq == -1)
	{
		perror("kqueue");
		exit(1);
	}

	int const webservAndCgi = 2;
	int const inOut = 2;
	int fds[NUM_REQUEST][webservAndCgi][inOut];

	// kqueue 이벤트 설정
	struct kevent event;
	std::vector<struct kevent> changeList;
	struct kevent eventList[NUM_REQUEST];

	/* Create two processs */
	for (int i = 0; i < NUM_REQUEST; i++)
	{
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets[i]) == -1)
		{
			perror("socketpair");
			return 1;
		}

		// 자식 프로세스의 readFdFromCgi를 Non-Blocking 모드로 설정
		if (setNonBlocking(sockets[i][0]) == -1)
		{
			exit(1);
		}
		/* Generates a new process */
		pid_t pid = fork();

		/* child */
		if (pid == 0)
		{
			close(sockets[i][WEBSERV]);

			int readLen;
			while (1)
			{
				readLen = read(sockets[i][CGI], buffer, bufferSize);
				if (readLen > 0)
					break;
			}

			srand(time(nullptr) * getpid());
			int waitTime = rand() % MIN_TIME + ADD_TIME;
			sleep(waitTime);

			buffer[readLen] = '\0';
			std::string buf = buffer;
			std::string msg = buf + "-response-";
			int writeLen = write(sockets[i][CGI], msg.c_str(), msg.length());
			// std::cout << msg << std::endl;
			/* Closes child */
			exit(0);
		}
		else
		{
			// Timer 적용
			changeEvents(changeList, pid, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, TIME_LIMIT_MS, sockets + i);
			close(sockets[i][CGI]);

			// send
			changeEvents(changeList, sockets[i][WEBSERV], EVFILT_READ, EV_ADD, 0, NULL, NULL);
			std::string msg = std::to_string(i) + "-send-";
			write(sockets[i][WEBSERV], msg.c_str(), msg.length());
			// std::cout << msg << std::endl;
		}
	}

	int newEvents;
	struct kevent* currEvent;
	int finishedCnt = 0;

	// while (1)
	// while (changeList.size())
	while (finishedCnt < NUM_REQUEST)
	{
		// 이벤트 감시 시작
		newEvents = kevent(kq, &changeList[0], changeList.size(), eventList, NUM_REQUEST, NULL);
		if (newEvents == -1)
		{
			perror("kevent");
			exit(1);
		}
		changeList.clear();
		for (int i = 0; i < newEvents; i++)
		{
			currEvent = &(eventList[i]);

			if (currEvent->filter == EVFILT_TIMER)
			{
				close(static_cast<int*>(currEvent->udata)[WEBSERV]);
				close(static_cast<int*>(currEvent->udata)[CGI]);

				kill(currEvent->ident, SIGKILL);
				finishedCnt++;
				std::cout << "\t\t\t\tTimeout " << currEvent->ident << " "
						  << "/" << NUM_REQUEST << std::endl;
			}
			else
			{
				ssize_t bytes = read(currEvent->ident, &buffer, bufferSize);

				if (bytes <= 0)
					continue;

				std::string buf(buffer, buffer + bytes);
				std::string msg = buf + "-receive!\n";
				write(STDOUT_FILENO, msg.c_str(), msg.length());

				finishedCnt++;
				std::cout << "\t\t\t\tFinished " << finishedCnt << "/" << NUM_REQUEST << std::endl;
				// std::cout << "\t\t\t\tFinished "
				//   << "/" << NUM_REQUEST << std::endl;
			}
		}
	}
	// kq 종료
	// WEBSERV 소켓 종료
	close(kq);
	for (int i = 0; i < NUM_REQUEST; ++i)
	{
		close(sockets[i][1]);
	}
}