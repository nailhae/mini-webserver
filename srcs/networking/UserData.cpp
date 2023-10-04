#include "UserData.hpp"
#include "ChangeList.hpp"
#include "cgi.hpp"
#include "Error.hpp"
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

static void	replaceToContent(std::string& contents, std::string& findString, std::string& replaceString)
{
	size_t	found = 0;

	found = contents.find(findString, found);
	if (found == std::string::npos)
		return ;
	else
	{
		contents.erase(found, findString.size());
		contents.insert(found, replaceString);
		found += replaceString.size();
	}
	return ;
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
		std::cout << "root: "<< Colors::BoldCyanString(mSetting.rootPath) << mUri << std::endl;
		mUri.insert(0, mSetting.rootPath);
	}
	else
	{
		std::cout << "root: "<< Colors::BoldCyanString(mServerPtr->rootPath) << mUri << std::endl;
		mUri.insert(0, mServerPtr->rootPath);
	}
	return (mUri);
}

void generateReturnResponse(int fd, int code, std::string& body)
{
	std::stringstream returnResponse;

	returnResponse << "HTTP/1.1 " << code << " message" << "\r\n";
	returnResponse << "location: " << body << "\r\n\r\n";
	// returnResponse << "Content-type: text\r\n\r\n" << body;
	write(fd, returnResponse.str().c_str(), returnResponse.str().size());
	close(fd);
}

int UserData::GenerateGETResponse(void)
{
	std::ifstream requestedFile;
	std::string extTemp;

	requestedFile.open(mUri, std::ios::binary);
	if (requestedFile.is_open() == false)
	{
		// 4XX error
		Error::Print("open failed: " + mUri);
		write(mFd, "HTTP/1.1 404 Not found\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE HTML><HTML><H1>404 ERROR<H1><HTML>", 115);
		close(mFd);
	}
	else
	{
		extTemp = mUri.substr(mUri.find_last_of('.') + 1);
		std::cout << Colors::BlueString("open success: ") << mUri << std::endl;
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

static int checkHeaderLength(std::stringstream& ss)
{
	std::string line;

	// ss.seekg(std::ios::beg); // 필요하지 않다면 빼기
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

	mHeaderFlag = checkHeaderLength(mReceived);
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
		if (ParseRequest(mReceived) == ERROR)
		{
			// GenerateErrorResponse();
			std::cout << "Error page 전송해야 함" << std::endl;
			return;
		}
		if (mStatusCode < 1)
		{
			mMethod->ResponseConfigSetup(*mServerPtr, mUri, mSetting);
		}
		if (300 <= mStatusCode && mStatusCode < 600)
		{
			// TODO 조기종료 시키기
			// generateReturnResponse(mFd, mSetting.returnPair.first, mSetting.returnPair.second);
			std::cout << "더 이상 서버 자원을 잡아먹을 필요가 없음. 얼른 보내고 끝내라." << std::endl;
			return ;
		}
		mUri = uriGenerator();
		std::cout << Colors::BoldCyan << "[Method] " << mMethod->GetType() << std::endl;
    if (mMethod->GetType() == GET && mSetting.bGetMethod == true)
		{
			GenerateGETResponse();
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
				std::cout << Colors::BoldCyan << "[body]" << mBody << std::endl;
				std::cout << Colors::BoldCyan << "[body]" << temp << std::endl;
			}
			GeneratePostResponse();
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
	cgi.initCgiEnv(mUri, mContentSize, mHeaders, mBody);
	size_t errorCode = 0;
	cgi.execute(errorCode);
	cgi.sendCgiBody(mBody);
	mResponse = cgi.readCgiResponse();

	SendToClient(mFd);
	return (0);
}