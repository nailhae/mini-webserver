#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#define NUM_REQUEST 5
#define WEBSERV 0
#define CGI_PROGRAM 1

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
	struct kevent eventList[8];

	for (int i = 0; i < NUM_REQUEST; i++)
	{

		if (pipe(fds[i][WEBSERV]) == -1)
		{
			perror("pipe");
			exit(1);
		}
		if (pipe(fds[i][CGI_PROGRAM]) == -1)
		{
			perror("pipe");
			exit(1);
		}
	}

	/* Create two processs */
	for (int i = 0; i < NUM_REQUEST; i++)
	{

		// write(Out)할 때는 목적지의 writeFd에 write.
		// read(In)할 때는 자신의 readFd에 read.
		int readFdFromWebserv = fds[i][WEBSERV][STDIN_FILENO];
		int writeFdToWebserv = fds[i][WEBSERV][STDOUT_FILENO];
		int readFdFromCgi = fds[i][CGI_PROGRAM][STDIN_FILENO];
		int writeFdToCgi = fds[i][CGI_PROGRAM][STDOUT_FILENO];

		/* Generates a new process */
		pid_t pid = fork();

		/* child */
		if (pid == 0)
		{

			close(writeFdToCgi);
			close(readFdFromWebserv);

			int readLen;
			while (1)
			{
				readLen = read(readFdFromCgi, buffer, bufferSize);
				if (readLen > 0)
					break;
			}

			srand(time(nullptr) * getpid());
			int waitTime = rand() % 3 + 0;
			sleep(waitTime);

			buffer[readLen] = '\0';
			std::string buf = buffer;
			std::string msg = buf + "-response-";
			write(writeFdToWebserv, msg.c_str(), msg.length());
			std::cout << msg << std::endl;
			/* Closes child */
			exit(0);
		}
		else
		{
			close(writeFdToWebserv);
			close(readFdFromCgi);

			// send
			changeEvents(changeList, readFdFromWebserv, EVFILT_READ, EV_ADD, 0, 0, NULL);
			std::string msg = std::to_string(writeFdToCgi) + "-send-";
			write(writeFdToCgi, msg.c_str(), msg.length());
			std::cout << msg << std::endl;
		}
	}

	int newEvents;
	struct kevent* currEvent;
	int finishedCnt = 0;

	// while (1)
	while (finishedCnt < NUM_REQUEST)
	{
		// 이벤트 감시 시작
		newEvents = kevent(kq, &changeList[0], changeList.size(), eventList, 8, NULL);
		if (newEvents == -1)
		{
			perror("kevent");
			exit(1);
		}
		changeList.clear();
		for (int i = 0; i < newEvents; i++)
		{
			currEvent = &(eventList[i]);

			ssize_t bytes = read(currEvent->ident, &buffer, bufferSize);

			/* If the ev.data.fd has bytes added print, else wait */
			if (bytes > 0)
			{
				buffer[bytes] = '\0';
				std::string buf = buffer;
				std::string msg = buf + "-receive!\n";
				write(STDOUT_FILENO, msg.c_str(), msg.length());

				close(fds[i][CGI_PROGRAM][STDOUT_FILENO]);
				close(fds[i][WEBSERV][STDIN_FILENO]);

				finishedCnt++;
				std::cout << "\tFinished " << finishedCnt << "/" << NUM_REQUEST << std::endl;
			}
		}
	}
}