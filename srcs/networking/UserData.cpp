#include "UserData.hpp"

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

int UserData::GenerateGETResponse(void)
{
	std::ifstream requestedFile;
	std::string extTemp;

	extTemp = mUri.substr(mUri.find('.') + 1);
	requestedFile.open("../../html" + mUri, std::ios::binary);
	if (requestedFile.is_open() == false)
	{
		// 4XX error
		Error::Print("open failed: ." + mUri);
		write(mFd,
			  "HTTP/1.1 404 Not found\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE "
			  "HTML><HTML><H1>404 ERROR<H1><HTML>",
			  115);
		close(mFd);
	}
	else
	{
		std::cout << Colors::BlueString("open success: ../../html") << mUri << std::endl;
		std::stringstream fileContent;
		requestedFile.seekg(0, std::ios::end);
		std::streampos fileSize = requestedFile.tellg();
		requestedFile.seekg(0, std::ios::beg);
		mResponse = "HTTP/1.1 200 OK\r\nContent-type: ";
		if (extTemp == "png")
			mResponse += "image/";
		else
			mResponse += "text/";
		mResponse += extTemp;
		mResponse += "\r\nContent-length: ";
		mResponse += std::to_string(fileSize);
		mResponse += "\r\n\r\n";
		fileContent << requestedFile.rdbuf();
		mResponse += fileContent.str();
		requestedFile.close();
		SendToClient(mFd);
	}
	return (0);
}

const std::stringstream& UserData::GetReceived(void) const
{
	return (mReceived);
}

const std::string& UserData::GetResponse(void) const
{
	return (mResponse);
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

void UserData::GenerateResponse(void)
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
	else if (mFillBodyFlag == true)
	{
		if (mBody.size() < mContentSize)
		{
			int i = 0;
			while (mBody.size() < mContentSize && i < BUFFER_SIZE)
			{
				mBody.push_back(mBuf[i]);
				i++;
			}
			std::cout << Colors::Red << "[bodysize]" << mBody.size() << " " << mContentSize << Colors::Reset
					  << std::endl;
			if (mBody.size() < mContentSize && i < BUFFER_SIZE)
				return;
			else
				GeneratePostResponse();
		}
	}
	else
	{
		if (ParseRequest(mReceived) == ERROR)
		{
			// GenerateErrorResponse();
			std::cout << "Error page 전송해야 함" << std::endl;
			return;
		}
		std::cout << Colors::BoldCyan << "[Method]" << mMethod->GetType() << std::endl;
		if (mMethod->GetType() == GET)
			GenerateGETResponse();
		else if (mMethod->GetType() == HEAD)
			std::cout << "HEAD response 전송해야 함." << std::endl;
		else if (mMethod->GetType() == POST)
		{
			std::cout << Colors::BoldCyan << "[mContentSize]" << mHeaders[CONTENT_LENGTH] << std::endl;
			mContentSize = strtol(mHeaders[CONTENT_LENGTH].c_str(), NULL, 10);
			mBody.reserve(mContentSize);
			if (mBody.size() < mContentSize)
			{
				int i = 0;
				while (mBody.size() < mContentSize && i < BUFFER_SIZE)
				{
					mBody.push_back(mBuf[i]);
					i++;
				}
				if (mBody.size() < mContentSize && i < BUFFER_SIZE)
				{
					mFillBodyFlag = true;
					return;
				}
				else
					GeneratePostResponse();
			}
		}
		else if (mMethod->GetType() == DELETE)
			std::cout << "DELETE response 전송해야 함." << std::endl;
	}
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
	mResponse.clear();
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
	len = write(fd, mResponse.c_str(), mResponse.size());
	if (len < 0)
		Error::Print("send()");
	InitUserData();
	return (len);
}

int UserData::GeneratePostResponse(void)
{
	Cgi cgi(mUri);
	cgi.initCgiEnv(mUri, mContentSize, mHeaders, mBody, mMethod->GetType());
	size_t errorCode = 0;
	std::cout << Colors::Magenta << "[send]"
			  << "send" << std::endl;
	cgi.execute(errorCode);
	std::cout << Colors::Magenta << "[read]"
			  << "read" << std::endl;
	cgi.sendCgiBody(mBody);
	mResponse = cgi.readCgiResponse();
	SendToClient(mFd);
	return (0);
}
