#include "UserData.hpp"

#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

#include "ChangeList.hpp"
#include "Error.hpp"
#include "MethodGet.hpp"
#include "MethodPost.hpp"
#include "WebServer.hpp"

UserData::UserData(int fd)
	: mFd(fd)
	, mSocketType(0)
	, mStatusCode(0)
	, mHeaderFlag(0)
	, mChunkedFlag(0)
	, mFillBodyFlag(0)
	, mContentSize(0)
	, mMethod(NULL)
{
}

UserData::~UserData(void)
{
}

void UserData::InitUserData(void)
{
	if (mMethod != NULL)
	{
		delete mMethod;
		mMethod = NULL;
	}
	memset(mBuf, 0, sizeof(mBuf));
	mStatusCode = 0;
	mHeaderFlag = 0;
	mChunkedFlag = 0;
	mFillBodyFlag = 0;
	mStatusText.clear();
	mUri.clear();
	mBody.clear();
	mReceived.clear();
	mHeaders.clear();
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
		std::cout << "\nroot: " << Colors::BoldCyanString(mSetting.rootPath) << mUri << std::endl;
		mUri.insert(0, mSetting.rootPath);
	}
	else
	{
		std::cout << "\nroot: " << Colors::BoldCyanString(mServerPtr->rootPath) << mUri << std::endl;
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

ServerBlock* UserData::GetServerPtr(void) const
{
	return (mServerPtr);
}

void UserData::SetServerPtr(ServerBlock* serverPtr)
{
	mServerPtr = serverPtr;
}

static int checkHeaderLength(std::vector<unsigned char>& received, int& flag)
{
	std::vector<unsigned char>::iterator pos = received.begin();
	std::string line;

	if (flag == true)
		return (true);
	// if (received.size() > 1024)
	// 	return (ERROR);
	for (std::vector<unsigned char>::iterator it = received.begin(); it != received.end();)
	{
		pos = std::find(pos, received.end(), '\n');
		if (pos == received.end())
		{
			flag = false;
			return (false);
		}
		line.assign(it, pos);
		if (line == "\r" || line == "")
		{
			flag = true;
			return (true);
		}
		pos += 1; // 현재 pos는 \n을 가리키고 있기 때문.
		it = pos; // 찾기 시작하는 위치 저장.
	}
	flag = false;
	return (false);
}

static int checkChunkedMessageEnd(std::vector<unsigned char>& received)
{
	if (received.size() > 7 && received[received.size() - 7] == '\r' && received[received.size() - 6] == '\n' &&
		received[received.size() - 5] == '0' && received[received.size() - 4] == '\r' &&
		received[received.size() - 3] == '\n' && received[received.size() - 2] == '\r' &&
		received[received.size() - 1] == '\n')
	{
		return (true);
	}
	else
	{
		// std::cout << "chunked on and on: " << mReceived.size() << std::endl;
		return (false);
	}
}

int UserData::preprocessGenResponse()
{
	if (ParseRequest(mReceived) == ERROR)
	{
		mMethod->GenerateErrorResponse(mStatusCode);
		return (1);
	}
	mMethod->ResponseConfigSetup(*mServerPtr, mUri, mSetting);
	if (300 <= mStatusCode && mStatusCode < 600)
	{
		mMethod->GenerateRedirectionResponse(mStatusCode, mSetting);
		return (1);
	}
	return (0);
}

void UserData::passBodyToPost(void)
{

	std::string body(mReceived.begin(), mReceived.end());
	mMethod->GenerateResponse(mUri, mSetting, mHeaders, body);
	std::cout << body.size() << "|body  contentSize|" << mContentSize << std::endl;
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	return;
}

void UserData::ReadRequest(void)
{
	std::string temp;

	if (mHeaderFlag == ERROR)
	{
		mMethod = new MethodGet(mFd);
		mMethod->GenerateErrorResponse(mStatusCode);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
		return;
	}
	else if (checkHeaderLength(mReceived, mHeaderFlag) == false)
	{
		return;
	}

	if (mChunkedFlag == true || mFillBodyFlag == true)
	{
		if (mFillBodyFlag == true && mReceived.size() < mContentSize)
		{
			return;
		}
		if (mChunkedFlag == true && checkChunkedMessageEnd(mReceived) == false)
		{
			return;
		}
		passBodyToPost();
		return;
	}

	if (preprocessGenResponse() == 1)
	{
		mMethod->GenerateErrorResponse(mStatusCode);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	}
	else
	{
		mUri = uriGenerator();
		if (mMethod->GetType() == POST)
		{
			if (mReceived.size() < mContentSize)
			{
				if (mSetting.bPostMethod == false)
				{
					mMethod->GenerateErrorResponse(405);
					WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
					WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
				}
				else if (mHeaders[TRANSFER_ENCODING] == "chunked")
				{
					if (checkChunkedMessageEnd(mReceived) == true)
					{
						passBodyToPost();
					}
					else
					{
						mChunkedFlag = true;
					}
				}
				else
				{
					mFillBodyFlag = true;
				}
			}
			else
			{
				passBodyToPost();
			}
			return;
		}
		else
		{
			mMethod->GenerateResponse(mUri, mSetting, mHeaders);
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
		}
	}
}

int UserData::RecvFromClient(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	mReceived.insert(mReceived.end(), &mBuf[0], &mBuf[len]);
	// std::string test(&mBuf[0], &mBuf[len]);
	// std::cout << Colors::BoldGreen;
	// for (std::string::iterator it = test.begin(); it != test.end(); it++)
	// {
	// 	std::cout << *it;
	// }
	// std::cout << Colors::Reset << std::endl;
	return (len);
}

int UserData::SendToClient(int fd)
{
	int len;
	int maxWrite;

	// std::cout << Colors::BoldCyan << "[Headers]" << Colors::Reset << std::endl;
	// for (std::map<int, std::string>::iterator it = mHeaders.begin(); it != mHeaders.end(); it++)
	// {
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// }

	// 1. 현재 보내주려는 본문의 길이 - 버퍼 사이즈
	// 2. 전송을 더 해야하는 경우 write 이벤트 발생시킨다.
	// 3. 전송을 다 한 경우 read 이벤트를 켜준다.

	if (mMethod->GetResponse().size() >= BUFFER_SIZE)
		maxWrite = BUFFER_SIZE;
	else
		maxWrite = mMethod->GetResponse().size();

	std::cout << mMethod->GetResponse().size() << std::endl;

	len = write(fd, mMethod->GetResponse().c_str(), maxWrite);
	len = write(1, mMethod->GetResponse().c_str(), maxWrite);
	if (len < 0)
	{
		Error::Print("send()");
		return (ERROR);
	}
	mMethod->EraseResponse(maxWrite);
	if (mMethod->GetResponse().size() <= 0)
	{
		std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
		InitUserData();
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_ENABLE, this);
	}
	else
	{
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	}
	return (len);
}
