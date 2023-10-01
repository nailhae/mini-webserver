#include "./UserData.hpp"
#include "./ChangeList.hpp"

UserData::UserData(int fd)
	: mFd(fd)
	, mMethod(-1)
	, mStatusCode(-1)
	, mHeaderFlag(0)
	, mFillBodyFlag(-1)
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
		std::cerr << Colors::RedString("open failed: ." + mUri) << std::endl;
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

int UserData::GetMethod(void) const
{
	return (mMethod);
}

int UserData::GetFd(void) const
{
	return (mFd);
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
			continue ;
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
		return ;
	}
	else if (mHeaderFlag == false)
	{
		return ;
	}
	else
	{
		if (mFillBodyFlag == -1 && ParseRequest(mReceived) == ERROR)
		{
			// GenerateErrorResponse();
			std::cout << "Error page 전송해야 함" << std::endl;
			return ;
		}
		std::getline(mReceived, temp, static_cast<char>(EOF));
		mBody += temp;
		if (mBody.size() < mContentSize)
		{
			return ;
		}
		if (mMethod == GET)
			GenerateGETResponse();
		else if (mMethod == HEAD)
			std::cout << "HEAD response 전송해야 함." << std::endl;
		else if (mMethod == POST)
			std::cout << "POST response 전송해야 함." << std::endl;
		else if (mMethod == DELETE)
			std::cout << "DELETE response 전송해야 함." << std::endl;
	}
}

int UserData::RecvFromClient(int fd)
{
	int len;

	len = read(fd, mBuf, BUFFER_SIZE);
	for (int i = 0; i < len; i++)
		mReceived << mBuf[i];
	return (len);
}

void UserData::InitUserData(void)
{
	mMethod = -1;
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
	std::cout <<Colors::BoldBlue <<  "\nstatus " << mStatusCode << ": " << mStatusText << std::endl;
	std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
	len = write(fd, mResponse.c_str(), mResponse.size());
	if (len < 0)
		std::cout << Colors::RedString("send() error") << std::endl;
	InitUserData();
	return (len);
}
