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

void generateReturnResponse(int fd, int code, std::string& body)
{
	std::stringstream returnResponse;

	returnResponse << "HTTP/1.1 " << code << " message"
				   << "\r\n";
	returnResponse << "location: " << body << "\r\n\r\n";
	// returnResponse << "Content-type: text\r\n\r\n" << body;
	write(fd, returnResponse.str().c_str(), returnResponse.str().size());
	close(fd);
}

int UserData::loadFolderContent(void)
{
	DIR* dirInfo = NULL;
	struct dirent* dirEntry = NULL;
	std::stringstream ss;

	dirInfo = opendir(mUri.c_str());
	if (dirInfo == NULL)
	{
		mStatusCode = 404;
		Error::Print(mUri + "Open Error");
		return (ERROR);
	}
	ss << "<!DOCTYPE HTML>\n<HTML>\n\t<head>\n\t\t<title>Index of " << mUri << "</title>\n"
	   << "<style>\n";
	ss << "body {font-family: Arial, sans-serif; background-color: #f4f4f4;margin: 0;padding: 0;}\n";
	ss << "header {background-color: #333;color: #fff;text-align: center;padding: 10px;}\n";
	ss << ".container {max-width: 600px;margin: 0 auto;padding: 20px;background-color: #fff;border-radius: "
		  "5px;box-shadow: 0 0 5px rgba(0, 0, 0, 0.3);}\n";
	ss << "u1 {list-style-type: none;padding: 0;}\n";
	ss << "li {margin-bottom: 10px;}\n";
	ss << ".file {color: #007bff; text-decoration: none;}";
	ss << ".file:hover {background-color: #F0FFF0;text-decoration: underline;}";
	ss << ".dir {color: #B22222; text-decoration: none;}";
	ss << ".dir:hover {background-color: #F0FFF0;text-decoration: underline;}";

	ss << "</style>"
	   << "\t</head>\n\t<body>\n\t\t<header><h1>Index of " << mUri
	   << "</h1></header>\n\t\t<div class=\"container\"><u1>";
	while ((dirEntry = readdir(dirInfo)) != NULL)
	{
		if (dirEntry->d_type == DT_DIR)
		{
			dirEntry->d_name[strlen(dirEntry->d_name)] = '/';
			ss << "\t\t\t<li><a class=\"dir\" href=\"" << dirEntry->d_name << "\">";
		}
		else
			ss << "\t\t\t<li><a class=\"file\" href=\"" << dirEntry->d_name << "\">";
		ss << dirEntry->d_name;
		ss << "</br></a></li>\n";
	}
	ss << "\t\t</a></div>\n\t</body>\n</HTML>";
	closedir(dirInfo);
	mBody = ss.str();
	std::ofstream fs("../autoIndex/test.html");
	fs << mBody;
	return (0);
}

int UserData::GenerateGETResponse(void)
{
	std::ifstream requestedFile;
	std::string extTemp;

	if (*(mUri.end() - 1) == '/') // 폴더에 대한 요청은 무조건 autoindex로
	{
		int result = 0;
		std::cout << "autoindex == " << mSetting.autoindex << std::endl;
		if (mSetting.autoindex == true)
		{
			result = loadFolderContent();
		}
		else
		{
			mStatusCode = 403;
			write(mFd,
				  "HTTP/1.1 403 Forbidden\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE "
				  "HTML><HTML><H1>403 ERROR<H1><HTML>",
				  115);
			close(mFd);
			return (0);
		}
		if (result == ERROR)
		{
			write(mFd,
				  "HTTP/1.1 404 Not Found\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE "
				  "HTML><HTML><H1>404 ERROR<H1><HTML>",
				  115);
			close(mFd);
			return (0);
		}
		else
		{
			mResponse = "HTTP/1.1 200 OK\r\nContent-type: ";
			mResponse += "text/html";
			mResponse += "\r\nContent-length: ";
			mResponse += std::to_string(mBody.size());
			mResponse += "\r\n\r\n";
			mResponse += mBody;
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
		}
	}
	else
	{
		struct stat fileInfo;

		if (stat(mUri.c_str(), &fileInfo) == ERROR)
		{
			Error::Print("Not found: " + mUri);
			write(mFd,
				  "HTTP/1.1 404 Not found\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE "
				  "HTML><HTML><H1>404 ERROR<H1><HTML>",
				  115);
			close(mFd);
		}
		else if (S_ISDIR(fileInfo.st_mode) == true)
		{
			Error::Print("directory can't open in file mode: " + mUri);
			write(mFd,
				  "HTTP/1.1 403 Forbidden\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE "
				  "HTML><HTML><H1>403 ERROR<H1><HTML>",
				  115);
			close(mFd);
		}
		requestedFile.open(mUri, std::ios::binary);
		if (requestedFile.is_open() == false)
		{
			Error::Print("open failed: " + mUri);
			write(mFd,
				  "HTTP/1.1 404 Not found\r\nContent-type: text/html\r\ncontent-length: 45\r\n\r\n<!DOCTYPE "
				  "HTML><HTML><H1>404 ERROR<H1><HTML>",
				  115);
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
			if (extTemp == "png" || extTemp == "ico")
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
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
		}
	}
	return (0);
}

static void sendErrorPage(int clientSocket, int responseCode)
{
	std::string firstLine;
	std::string htmlContent;

	switch (responseCode)
	{
	case 403:
		firstLine = "HTTP/1.1 403 Forbidden\r\n";
		htmlContent = "<html>\n"
					  "  <head>\n"
					  "    <title>403 Forbidden</title>\n"
					  "  </head>\n"
					  "  <body>\n"
					  "    <h1>403 Forbidden</h1>\n"
					  "    <p>You do not have permission to access this resource.</p>\n"
					  "  </body>\n"
					  "</html>";
		break;

	case 404:
		firstLine = "HTTP/1.1 404 Not Found\r\n";
		htmlContent = "<html>\n"
					  "  <head>\n"
					  "    <title>404 Not Found</title>\n"
					  "  </head>\n"
					  "  <body>\n"
					  "    <h1>404 Not Found</h1>\n"
					  "    <p>The requested page was not found.</p>\n"
					  "  </body>\n"
					  "</html>";
		break;

	case 500:
		firstLine = "HTTP/1.1 500 Internal Server Error\r\n";
		htmlContent = "<html>\n"
					  "  <head>\n"
					  "    <title>500 Internal Server Error</title>\n"
					  "  </head>\n"
					  "  <body>\n"
					  "    <h1>500 Internal Server Error</h1>\n"
					  "    <p>There was an internal server error while processing your request.</p>\n"
					  "  </body>\n"
					  "</html>";
		break;

	default:
		firstLine = "HTTP/1.1 501 Not Implemented\r\n";
		htmlContent = "<html>\n"
					  "  <head>\n"
					  "    <title>501 Not Implemented</title>\n"
					  "  </head>\n"
					  "  <body>\n"
					  "    <h1>501 Not Implemented</h1>\n"
					  "    <p>The requested functionality is not implemented on this server.</p>\n"
					  "  </body>\n"
					  "</html>";
		break;
	}

	std::ostringstream response;
	response << firstLine << "Content-Type: text/html\r\n"
			 << "Content-Length: " << htmlContent.length() << "\r\n"
			 << "\r\n"
			 << htmlContent;

	write(clientSocket, response.str().c_str(), response.str().length());
}

int UserData::GenerateDeleteResponse(void)
{
	if (access(mUri.c_str(), F_OK) == ERROR)
	{
		Error::Print("404 Not Found");
		sendErrorPage(mFd, 404);
		close(mFd);
		return ERROR;
	}
	if (access(mUri.c_str(), W_OK) == ERROR)
	{
		Error::Print("403 Forbidden");
		sendErrorPage(mFd, 403);
		close(mFd);
		return ERROR;
	}
	if (std::remove(mUri.c_str()) == ERROR)
	{
		Error::Print("500 Internal Server Error");
		sendErrorPage(mFd, 500);
		close(mFd);
		return ERROR;
	}
	std::cout << "Success remove file" << std::endl;
	// TODO apply date method
	write(mFd,
		  "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\nContent-length: 76\r\nDate: Wed, 21 Oct 2015 07:28:00 "
		  "GMT<html>  <body>    <h1>File deleted.</h1>  </body></html>",
		  154);
	close(mFd);

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
			// generateReturnResponse(mFd, mSetting.returnPair.first, mSetting.returnPair.second);
			std::cout << "더 이상 서버 자원을 잡아먹을 필요가 없음. 얼른 보내고 끝내라." << std::endl;
			return;
		}
		// 3. 설정을 실제 open 해야 할 uri를 구성
		mUri = uriGenerator();
		// 4. 각 method에 따라 응답 메시지 생성
		std::cout << Colors::BoldCyan << "[Method] " << mMethod->GetType() << std::endl;
		if (mMethod->GetType() == GET && mSetting.bGetMethod == true)
		{
			// mMethod->GenerateResponse();
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
				mFillBodyFlag = true;
				return ;
			}
			GeneratePostResponse();
		}
		else if (mMethod->GetType() == DELETE)
			GenerateDeleteResponse();
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
	len = write(1, mResponse.c_str(), mResponse.size());
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

	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, this);
	// SendToClient(mFd);
	return (0);
}