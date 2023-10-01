#include "./UserData.hpp"
#include "./ChangeList.hpp"

static void	trimWhiteSpace(std::string& target)
{
	size_t	start = 0;

	while (start < target.size() && isspace(target[start]) == true)
	{
		start += 1;
	}
	target.erase(0, start);

	size_t	end = target.size() - 1;
	while (end > 0 && isspace(target[end]) == true)
	{
		end -= 1;
	}
	target.erase(end + 1);
}

static int validHeader(std::string& content)
{
	for (std::string::iterator it = content.begin(); it != content.end(); it++)
	{
		if (isupper(*it) == true)
			*it -= 32;
	}
	if (content == "connection")
		return (CONNECTION);
	else if (content == "content-type")
		return (CONTENT_TYPE);
	else if (content == "content-length")
		return (CONTENT_LENGTH);
	else if (content == "cache-control")
		return (CACHE_CONTROL);
	else if (content == "if-none-match")
		return (IF_NONE_MATCH);
	else if (content == "if-modified-since")
		return (IF_MODIFIED_SINCE);
	else
		return (NONE);
}

int UserData::ParseHeader(std::string& field)
{
	std::istringstream ss(field);
	std::string header;
	std::string value;
	size_t		pos;
	int			headerKey;

	std::getline(ss, header, ':');
	trimWhiteSpace(header);
	headerKey = validHeader(header);
	if (headerKey == NONE)
		return (0);
	else
	{
		std::getline(ss, value);
		trimWhiteSpace(value);
		// ""처리 할 일이 있으면 여기서 trim 해주기.
		if (value.size() == 0)
		{
			mStatusCode = 400; 
			mStatusText = "Bad Request";
			return (ERROR);
		}
		mHeaders[headerKey] = value;
	}
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
		mStatusCode = 405; 
		mStatusText = "Method is not allowed";
		// 헤더에 Allow: GET, POST, DELETE 추가해야 함.
		return (ERROR);
	}
	std::getline(request, mUri, ' ');
	std::getline(request, temp, '\n');
	if (*(temp.end() - 1) == '\r')
		temp.erase(temp.size() - 1);
	if (temp != "HTTP/1.1")
	{
		mStatusCode = 505; 
		mStatusText = "HTTP Version Not Supported";
		return (ERROR);
	}

	while (1)
	{
		std::getline(request, temp, '\n');
		if (request.eof())
			return (0);
		else if (request.fail())
		{
			mStatusCode = 500; 
			mStatusText = "Internal Server Error";
			return (ERROR);
		}
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
	, mStatusCode(0)
	, mHeaderFlag(0)
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
	{
		mStatusCode = 416;
		mStatusText = "Requested Range Not Satisfiable";
		return (ERROR);
	}	
	else if (mHeaderFlag == false)
		return (0);
	else
	{
		if (ParseRequest(mReceived) == ERROR)
			return (ERROR);
		if (mMethod == GET)
			GenerateGETResponse();
		else if (mMethod == POST)
			GeneratePostResponse();
	}
	return (0);
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
	mStatusText.clear();
	mUri.clear();
	mReceived.str("");
	mResponse.clear();
	mHeaders.clear();
}

int UserData::SendToClient(int fd)
{
	size_t len;

	std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
	len = write(fd, mResponse.c_str(), mResponse.size());
	if (len < 0)
	{
		std::cout << Colors::RedString("send() error") << std::endl;
		exit(1);
	}
	InitUserData();
	return (len);
}

int UserData::ReadCgiResponse(void) {
	char buffer[BUFFER_SIZE];
	int readBuffer = read(cgi.pipeOut[0], buffer, BUFFER_SIZE);

}