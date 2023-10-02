#include "./UserData.hpp"
#include "./ChangeList.hpp"
#include "Error.hpp"

UserData::UserData(int fd)
	: mFd(fd)
	, mStatusCode(-1)
	, mHeaderFlag(0)
	, mFillBodyFlag(-1)
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

	if (mUri == "/")
		mUri = "/index.html";
	extTemp = mUri.substr(mUri.find('.') + 1);
	requestedFile.open("." + mUri, std::ios::binary);
	if (requestedFile.is_open() == false)
	{
		// 4XX error
		Error::Print("open failed: ." + mUri);
		write(mFd, "HTTP/1.1 404 Not found\r\n\r\n", 26);
	}
	else
	{
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

const ServerBlock UserData::GetServerPtr(void) const
{
	return (*mServerPtr);
}

void UserData::SetSocketType(ServerBlock* serverPtr)
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
		if (mFillBodyFlag == -1 && ParseRequest(mReceived) == ERROR)
		{
			// GenerateErrorResponse();
			std::cout << "Error page 전송해야 함" << std::endl;
			return;
		}
		std::getline(mReceived, temp, static_cast<char>(EOF));
		mBody += temp;
		if (mBody.size() < mContentSize)
		{
			return;
		}
		if (mMethod->GetType() == GET)
			GenerateGETResponse();
		else if (mMethod->GetType() == HEAD)
			std::cout << "HEAD response 전송해야 함." << std::endl;
		else if (mMethod->GetType() == POST)
			std::cout << "POST response 전송해야 함." << std::endl;
		else if (mMethod->GetType() == DELETE)
			std::cout << "DELETE response 전송해야 함." << std::endl;
	}
}

int UserData::RecvFromClient(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	for (int i = 0; i < len; i++)
		mReceived << mBuf[i];
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
