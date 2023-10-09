#include "UserData.hpp"

#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

#include "ChangeList.hpp"
#include "Error.hpp"
#include "MethodGet.hpp"
#include "WebServer.hpp"

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
	if (*(mUri.end() - 1) == '/')
	{
		mUri += mSetting.index;
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

const std::vector<unsigned char>& UserData::GetReceived(void) const
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

static int checkHeaderLength(std::vector<unsigned char>& received, int flag)
{
	std::vector<unsigned char>::iterator pos = received.begin();
	std::string line;

	if (flag == true)
		return (true);
	// if (received.size() > 1024)
	// return (ERROR);
	for (std::vector<unsigned char>::iterator it = received.begin(); it != received.end();)
	{
		pos = std::find(pos, received.end(), '\n');
		if (pos == received.end())
			return (false);
		line.assign(it, pos);
		if (line == "\r" || line == "")
			return (true);
		pos += 1; // 현재 pos는 \n을 가리키고 있기 때문.
		it = pos; // 찾기 시작하는 위치 저장.
	}
	return (false);
}

void UserData::ReadRequest(void)
{
	std::string temp;

	mHeaderFlag = checkHeaderLength(mReceived, mHeaderFlag);
	if (mHeaderFlag == ERROR)
	{
		mMethod = new MethodGet(mFd);
		mMethod->GenerateErrorResponse(mStatusCode);
	}
	else if (mHeaderFlag == false)
	{
		return;
	}

	if (mFillBodyFlag == true)
	{
		if (mReceived.size() < mContentSize)
		{
			mFillBodyFlag = true;
			return;
		}
		mMethod->GenerateResponse(mUri, mSetting, mHeaders);
	}
	else
	{
		// 1. 요청 파싱 // 400~
		if (ParseRequest(mReceived) == ERROR)
		{
			mMethod->GenerateErrorResponse(mStatusCode);
			mContentSize = 0;
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
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
			// TODO 에러와 함께 처리가 되고 있는지 보아야 함.
			mMethod->GenerateRedirectionResponse(mStatusCode, mSetting);
		}
		else
		{
			// 3. 설정을 실제 open 해야 할 uri를 구성
			mUri = uriGenerator();
			// 4. 각 method에 따라 응답 메시지 생성
			std::cout << Colors::BoldCyan << "[Method] " << mMethod->GetType() << std::endl;
			if (mMethod->GetType() == GET && mSetting.bGetMethod == true)
			{
				mMethod->GenerateResponse(mUri, mSetting, mHeaders);
			}
			else if (mMethod->GetType() == HEAD && mSetting.bHeadMethod == true)
			{
				mMethod->GenerateErrorResponse(405);
				// mMethod->GenerateResponse(mUri, mSetting, mHeaders);
			}
			else if (mMethod->GetType() == POST)
			{
				std::cout << Colors::BoldCyan << "[mContentSize]" << mHeaders[CONTENT_LENGTH] << std::endl;
				std::cout << Colors::BoldCyan << "[body]" << mContentSize << std::endl;
				std::cout << "length: " << mContentSize << std::endl;
				if (mReceived.size() < mContentSize)
				{
					if (mSetting.bPostMethod == false)
						mMethod->GenerateErrorResponse(405);
					else if (mHeaders[TRANSFER_ENCODING] == "chunked")
					{
						int num = 0;

						for (std::vector<unsigned char>::iterator it = mReceived.begin(); it != mReceived.end(); it++)
						{
							if (isdigit(*it) == true)
							{
								num += *it - '0';
								num *= 10;
							}
							else
							{
								break;
							}
						}
						mContentSize = num;
						if (num == 0)
							mMethod->GenerateErrorResponse(204);
					}
					else if (mContentSize == 0)
						mMethod->GenerateErrorResponse(411);
					else
					{
						mFillBodyFlag = true;
						return;
					}
				}
				else
					mMethod->GenerateResponse(mUri, mSetting, mHeaders);
			}
			else if (mMethod->GetType() == DELETE && mSetting.bDeleteMethod == true)
				mMethod->GenerateResponse(mUri, mSetting, mHeaders);
			else
			{
				mMethod->GenerateErrorResponse(405);
			}
		}
	}
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
}

int UserData::RecvFromClient(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	mReceived.insert(mReceived.end(), &mBuf[0], &mBuf[len]);
	return (len);
}

void UserData::InitUserData(void)
{
	if (mMethod != NULL)
	{
		delete mMethod;
		mMethod = NULL;
	}
	memset(mBuf, 0, sizeof(mBuf));
	mStatusCode = -1;
	mHeaderFlag = -1;
	mStatusCode = -1;
	mFillBodyFlag = -1;
	mStatusText.clear();
	mUri.clear();
	mBody.clear();
	mReceived.clear();
	mHeaders.clear();
}

int UserData::SendToClient(int fd)
{
	size_t len;

	// test code
	std::cout << Colors::BoldCyan << "[Headers]" << Colors::Reset << std::endl;
	for (std::map<int, std::string>::iterator it = mHeaders.begin(); it != mHeaders.end(); it++)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "\n" << std::endl;
	len = write(fd, mMethod->GetResponse().c_str(), mMethod->GetResponse().size());
	len = write(1, mMethod->GetResponse().c_str(), mMethod->GetResponse().size());
	// TODO 얘도 나눠서 써야 함.
	// WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	if (len < 0)
		Error::Print("send()");
	std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
	InitUserData();
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_ENABLE, this);
	return (len);
}
