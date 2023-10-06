#include "UserData.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include "ChangeList.hpp"
#include "Error.hpp"
#include "WebServer.hpp"
#include "cgi.hpp"

UserData::UserData(int fd)
	: mFd(fd)
	, mSocketType(-1)
	, mStatusCode(-1)
	, mHeaderFlag(0)
	, mFillBodyFlag(-1)
	, mContentSize(0)
	, mMethod(NULL)
{
}

UserData::~UserData(void)
{
}

static void replaceToContent(std::string& contents, std::string& findString, std::string& replaceString)
{
	size_t found = 0;

	found = contents.find(findString, found);
	if (found == std::string::npos)
		return;
	else
	{
		contents.erase(found, findString.size());
		contents.insert(found, replaceString);
		found += replaceString.size();
	}
	return;
}

std::string UserData::uriGenerator(void)
{
	std::string result;

	if (mSetting.alias.size() > 0)
	{
		replaceToContent(mUri, mSetting.uri, mSetting.alias);
		return (mUri);
	}
	if (mUri[0] == '/')
	{
		mUri.erase(0, 1);
		if (mUri.size() == 0)
			mUri = mSetting.index;
	}
	if (mSetting.rootPath.size() > 0)
	{
		std::cout << "root: " << Colors::BoldCyanString(mSetting.rootPath) << mUri << std::endl;
		mUri.insert(0, mSetting.rootPath);
	}
	else
	{
		std::cout << "root: " << Colors::BoldCyanString(mServerPtr->rootPath) << mUri << std::endl;
		mUri.insert(0, mServerPtr->rootPath);
	}
	return (mUri);
}

int UserData::GenerateDeleteResponse(void)
{
	if (access(mUri.c_str(), F_OK) == ERROR)
	{
		Error::Print("404 Not Found");
		mMethod->GenerateErrorResponse(404);
		return ERROR;
	}
	if (access(mUri.c_str(), W_OK) == ERROR)
	{
		Error::Print("403 Forbidden");
		mMethod->GenerateErrorResponse(403);
		return ERROR;
	}
	if (std::remove(mUri.c_str()) == ERROR)
	{
		Error::Print("500 Internal Server Error");
		mMethod->GenerateErrorResponse(500);
		return ERROR;
	}
	std::cout << "Success remove file" << std::endl;
	// TODO apply date method
	write(mFd,
		  "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\nContent-length: 76\r\nDate: Wed, 21 Oct 2015 07:28:00 "
		  "GMT<html>  <body>    <h1>File deleted.</h1>  </body></html>",
		  154);

	return (0);
}

const std::stringstream& UserData::GetReceived(void) const
{
	return (mReceived);
}

const AMethod& UserData::GetMethod(void) const
{
	return (*mMethod);
}

int UserData::GetFd(void) const
{
	return (mFd);
}

LocationBlock& UserData::Setting(void)
{
	return (mSetting);
}

const std::string& UserData::GetUri(void) const
{
	return (mUri);
}

int UserData::GetSocketType(void) const
{
	return (mSocketType);
}

void UserData::SetSocketType(int socketType)
{
	mSocketType = socketType;
}

const ServerBlock* UserData::GetServerPtr(void) const
{
	return (mServerPtr);
}

void UserData::SetServerPtr(const ServerBlock* serverPtr)
{
	mServerPtr = serverPtr;
}

static int checkHeaderLength(std::stringstream& ss, int flag)
{
	std::string line;

	// ss.seekg(std::ios::beg); // 필요하지 않다면 빼기
	if (flag == true)
		return (true);
	while (1)
	{
		std::getline(ss, line);
		if (ss.eof() == true)
			return (false);
		else if (line == "\r" || line == "")
			return (true);
		else if (ss.tellg() > 1024)
			return (ERROR);
		else
			continue;
	}
}

void UserData::ReadResponse(void)
{
	std::string temp;

	mHeaderFlag = checkHeaderLength(mReceived, mHeaderFlag);
	if (mHeaderFlag == ERROR)
	{
		mStatusCode = 416;
		mStatusText = "Requested Range Not Satisfiable";
		return;
	}
	else if (mHeaderFlag == false)
	{
		return;
	}
	else
	{
		// 1. 요청 파싱 // 400~
		if (ParseRequest(mReceived) == ERROR)
		{
			// GenerateErrorResponse();
			std::cout << "Error page 전송해야 함" << std::endl;
			return;
		}
		// 2. 요청에서 문제가 없을 경우 최하위 노드 찾아서 설정 값 받아오기
		if (mStatusCode < 1) // 초기값
		{
			mMethod->ResponseConfigSetup(*mServerPtr, mUri, mSetting);
		}
		// 2-1. 이미 상태코드가 정의가 된 경우 종료
		if (300 <= mStatusCode && mStatusCode < 600)
		{
			mMethod->GenerateRedirectionResponse(mStatusCode, mSetting); //TODO 에러와 함께 처리가 되고 있는지 보아야 함.
		}
		// 3. 설정을 실제 open 해야 할 uri를 구성
		mUri = uriGenerator();
		// 4. 각 method에 따라 응답 메시지 생성
		std::cout << Colors::BoldCyan << "[Method] " << mMethod->GetType() << std::endl;
		if (mMethod->GetType() == GET && mSetting.bGetMethod == true)
		{
			mMethod->GenerateResponse(mUri, mSetting, mHeaders);
		}
		else if (mMethod->GetType() == HEAD && mSetting.bHeadMethod == true)
			std::cout << "HEAD response 전송해야 함." << std::endl;
		else if (mMethod->GetType() == POST && mSetting.bPostMethod == true)
		{
			std::cout << Colors::BoldCyan << "[mContentSize]" << mHeaders[CONTENT_LENGTH] << std::endl;
			mContentSize = strtol(mHeaders[CONTENT_LENGTH].c_str(), NULL, 10);
			std::cout << Colors::BoldCyan << "[body]" << mContentSize << std::endl;
			if (mBody.size() < mContentSize)
			{
				std::getline(mReceived, temp, static_cast<char>(EOF));
				mBody += temp;
				mFillBodyFlag = true;
				return;
			}
			GeneratePostResponse();
		}
		else if (mMethod->GetType() == DELETE && mSetting.bDeleteMethod == true)
			GenerateDeleteResponse();
		else
			mMethod->GenerateErrorResponse(403);
	}
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
}

int UserData::RecvFromClient(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	for (int i = 0; i < len; i++)
	{
		mReceived << mBuf[i];
		std::cout << mBuf[i];
	}
	std::cout << std::endl;
	return (len);
}

void UserData::InitUserData(void)
{
	if (mMethod != NULL)
	{
		delete mMethod;
		mMethod = NULL;
	}
	mStatusCode = -1;
	mHeaderFlag = -1;
	mStatusCode = -1;
	mFillBodyFlag = -1;
	mStatusText.clear();
	mUri.clear();
	mReceived.str("");
	mHeaders.clear();
}

int UserData::SendToClient(int fd)
{
	size_t len;

	// test code
	std::cout << Colors::BoldCyan << "[Headers]" << std::endl;
	for (std::map<int, std::string>::iterator it = mHeaders.begin(); it != mHeaders.end(); it++)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << Colors::BoldBlue << "\nstatus " << mStatusCode << ": " << mStatusText << std::endl;
	std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
	len = write(fd, mMethod->GetResponse().c_str(), mMethod->GetResponse().size());
	len = write(1, mMethod->GetResponse().c_str(), mMethod->GetResponse().size());
	if (len < 0)
		Error::Print("send()");
	InitUserData();
	return (len);
}

int UserData::GeneratePostResponse(void)
{
	Cgi cgi(mUri);
	cgi.initCgiEnv(mUri, mContentSize, mHeaders, mBody);
	size_t errorCode = 0;
	cgi.execute(errorCode);
	cgi.sendCgiBody(mBody);
	// mResponse = cgi.readCgiResponse();

	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	// SendToClient(mFd);
	return (0);
}