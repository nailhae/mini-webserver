#pragma once

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "AMethod.hpp"
#include "dataSet.hpp"

#define BUFFER_SIZE 1024

class AMethod;

class UserData
{
public:
	UserData(int fd);
	~UserData(void);
	void InitUserData(void);
	const std::vector<unsigned char>& GetReceived(void) const;
	const std::string& GetResponse(void) const;
	const std::string& GetUri(void) const;

	const AMethod& GetMethod(void) const;
	int GetFd(void) const;
	int GetSocketType(void) const;
	void SetSocketType(int socketType);
	const ServerBlock* GetServerPtr(void) const;
	void SetServerPtr(const ServerBlock* serverPtr);
	LocationBlock& Setting(void);
	std::string uriGenerator(void);
	void ReadRequest(void);
	int GenerateDeleteResponse(void);
	int preprocessGenResponse(void);
	void passBodyToPost(void);
	int ParseRequest(std::vector<unsigned char>& request);
	int ParseFirstLine(std::string& request);
	int ParseHeaderKey(std::string& headerKey);
	int ParseOneLine(std::string& oneLine);
	int ParseHeaderValue(int headerKey, std::string& field);
	int RecvFromClient(void);
	int SendToClient(int fd);
	int GeneratePostResponse(void);
	int loadFolderContent(void);

	std::vector<unsigned char> mReceived;
	std::vector<unsigned char> mBody;

private:
	UserData(void);
	UserData(const UserData& rhs);
	UserData& operator=(const UserData& rhs);

	int mFd;
	char mBuf[BUFFER_SIZE];
	int mSocketType;
	int mStatusCode;
	int mHeaderFlag;
	int mChunkedFlag;
	int mFillBodyFlag;
	size_t mContentSize;
	LocationBlock mSetting;
	std::string mStatusText;
	std::string mUri;
	std::map<int, std::string> mHeaders;
	const ServerBlock* mServerPtr;
	AMethod* mMethod;
};
