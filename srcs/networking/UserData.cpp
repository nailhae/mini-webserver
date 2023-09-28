#include "./UserData.hpp"
#include "./ChangeList.hpp"

int UserData::ParseHeader(std::string& field)
{
	std::istringstream ss(field);
	std::string header;
	std::string value;
	/**
	 * @brief 의사코드
	 * : 까지 읽음
	 * ; 까지 읽음
	 * 각 토큰별 제거
	 */
	return (0);
}

int UserData::ParseRequest(std::stringstream& request)
{
	std::string temp;

	request.seekg(0);
	std::getline(request, temp, ' ');
	if (temp == "GET")
		mMethod = GET;
	else if (temp == "POST")
		mMethod = POST;
	else if (temp == "DELETE")
		mMethod = DELETE;
	else
	{
		mMethod = ERROR;
		std::cout << "Method Error" << std::endl;
		return (ERROR);
	}
	std::getline(request, mUri, ' ');
	std::getline(request, temp, '\n');
	if (*(temp.end() - 1) == '\r')
		temp.erase(temp.size() - 1);
	if (temp != "HTTP/1.1")
	{
		std::cout << temp << " version Error" << std::endl;
		return (ERROR);
	}

	while (1)
	{
		std::getline(request, temp, '\n');
		if (request.eof())
			return (0);
		else if (request.fail()) // 헤더가 너무 크다는 것.
			return (ERROR);
		else if (*(temp.end() - 1) == '\r')
			temp.erase(temp.size() - 1);
		if (temp == "")
			return (0);
		if (ParseHeader(temp) == ERROR)
			return (ERROR);
	}
	return (ERROR);
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

UserData::UserData(int fd)
	: mFd(fd)
	, mMethod(-1)
	, mStatus(0)
	, mHeaderFlag(0)
	, mReceived(std::string())
	, mResponse(std::string())
{
}

UserData::~UserData(void)
{
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

static int checkReceivedHeader(std::stringstream& ss)
{
	std::string line;

	ss.seekg(0, std::ios::end);
	if (ss.tellg() >= 1024)
		return (true);
	ss.seekg(0, std::ios::beg);
	while (1)
	{
		std::getline(ss, line, '\n');
		if (ss.eof() == true)
			return (false);
		else if (line == "" || line == "\r")
			return (true);
		else
			continue;
	}
}

int UserData::GenerateResponse(void)
{
	mHeaderFlag = checkReceivedHeader(mReceived);
	if (mHeaderFlag == ERROR)
		return (ERROR);
	else if (mHeaderFlag == false)
		return (0);
	else
	{
		if (ParseRequest(mReceived) == ERROR)
			return (ERROR);
		if (mMethod == GET)
			GenerateGETResponse();
	}
	return (0);
}

int UserData::RecvFromClient(int fd)
{
	int len;

	len = read(fd, mBuf, BUFFER_SIZE);
	std::cout << Colors::BoldBlue << "received message from client " << fd << "\n" << std::endl;
	for (int i = 0; i < len; i++)
	{
		std::cout << mBuf[i];
		mReceived << mBuf[i];
	}
	std::cout << Colors::Reset << std::endl;
	return (len);
}

void UserData::InitUserData(void)
{
	mMethod = -1;
	mStatus = -1;
	mHeaderFlag = -1;
	mUri.clear();
	mReceived.str("");
	mResponse.clear();
}

int UserData::SendToClient(int fd)
{
	size_t len;

	std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
	len = write(fd, mResponse.c_str(), mResponse.size()); // 큰 파일도 한 번에 보낼 수 있나?
	len = write(1, mResponse.c_str(), mResponse.size());  // 큰 파일도 한 번에 보낼 수 있나?
	if (len < 0)
	{
		std::cout << Colors::RedString("send() error") << std::endl;
		exit(1);
	}
	InitUserData();
	return (len);
}
