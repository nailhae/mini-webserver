#pragma once

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <signal.h>
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
	const std::string& GetUri(void) const;

	const AMethod& GetMethod(void) const;
	int GetFd(void) const;
	int GetSocketType(void) const;
	void SetSocketType(int socketType);
	ServerBlock* GetServerPtr(void) const;
	void SetServerPtr(ServerBlock* serverPtr);
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
	int RecvFromCgi(void);
	int SendToClient(int fd);
	int SendToClientPostResponse(int fd);
	int SendToCgi(void);
	void ClearBody(void);
	void ClearReceived(void);
	void GeneratePostResponse(int status);
	int loadFolderContent(void);
	int GetPid(void) const;
	void HandlingMethodPost(void);
	void CheckReceiveAll(void);
	UserData* GetClientUdata(void) const;
	void SetClientUdataNULL(void);
	void SetCgiEvent(void);

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
	int mPostFlag;
	int mPid;
	size_t mContentSize;
	LocationBlock mSetting;
	std::string mUri;
	std::map<int, std::string> mHeaders;
	std::vector<unsigned char>* mReceived;
	std::vector<unsigned char>* mBody;
	AMethod* mMethod;
	ServerBlock* mServerPtr;
	UserData* mClientUdata;
};
