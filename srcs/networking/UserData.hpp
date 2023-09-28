#pragma once
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#define BUFFER_SIZE 1024

class UserData
{
public:
	UserData(int fd);
	~UserData(void);
	void InitUserData(void);
	const std::stringstream& GetReceived(void) const;
	const std::string& GetResponse(void) const;
	int GetMethod(void) const;
	int GetFd(void) const;
	int GenerateResponse(void);
	int GenerateGETResponse(void);
	int ParseRequest(std::stringstream& request);
	int ParseHeader(std::string& field);
	int RecvFromClient(int fd);
	int SendToClient(int fd);

private:
	UserData(void);
	UserData(const UserData& rhs);
	UserData& operator=(const UserData& rhs);

	int mFd;
	char mBuf[BUFFER_SIZE];
	int mMethod;
	int mStatus;
	int mHeaderFlag;
	std::string mUri;
	std::stringstream mReceived;
	std::string mResponse;
};