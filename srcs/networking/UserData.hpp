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
#include <map>

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
	void GenerateResponse(void);
	int GenerateGETResponse(void);
	int ParseRequest(std::stringstream& request);
	int ParseFirstLine(std::stringstream& request);
	int ParseHeaderKey(std::string& headerKey);
	int ParseOneLine(std::string& oneLine);
	int ParseHeaderValue(int headerKey, std::string& field);
	int RecvFromClient(int fd);
	int SendToClient(int fd);
	int ReadCgiResponse(void);
	int GeneratePostResponse(void);

	std::stringstream mReceived;
	std::string mBody;
private:
	UserData(void);
	UserData(const UserData& rhs);
	UserData& operator=(const UserData& rhs);

	int mFd;
	char mBuf[BUFFER_SIZE];
	int mMethod;
	int mStatusCode;
	int mHeaderFlag;
	int mFillBodyFlag;
	int mContentSize;
	std::string mStatusText;
	std::string mUri;
	std::string mResponse;
	std::map<int, std::string> mHeaders;
};