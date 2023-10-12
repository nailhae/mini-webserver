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
	, mPostFlag(0)
	, mPid(0)
	, mContentSize(0)
	, mReceived(new std::vector<unsigned char>)
	, mBody(NULL)
	, mMethod(NULL)
	, mServerPtr(NULL)
	, mClientUdata(NULL)
{
}

UserData::~UserData(void)
{
	if (mMethod != NULL)
	{
		delete mMethod;
		mMethod = NULL;
	}
	if (mSocketType != CGI_SOCKET && mBody != NULL)
	{
		delete mBody;
		mBody = NULL;
	}
	if (mSocketType != CGI_SOCKET && mReceived != NULL)
	{
		delete mReceived;
		mReceived = NULL;
	}
}

void UserData::InitUserData(void)
{
	if (mMethod != NULL)
	{
		delete mMethod;
		mMethod = NULL;
	}
	memset(&mSetting, 0, sizeof(LocationBlock));
	memset(mBuf, 0, sizeof(mBuf));
	mStatusCode = 0;
	mHeaderFlag = 0;
	mChunkedFlag = 0;
	mFillBodyFlag = 0;
	mPid = 0;
	mPostFlag = 0;
	mContentSize = 0;
	mStatusText.clear();
	mUri.clear();
	if (mBody != NULL)
		mBody->clear();
	mReceived->clear();
	mHeaders.clear();
	mHeaders.clear();
	mClientUdata = NULL;
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
	if (*(mUri.end() - 1) == '/' && mSetting.autoindex == false)
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
	return (*mReceived);
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
	if (received.size() > 1024)
	{

		return (ERROR);
	}
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

	if (ParseRequest(*mReceived) == ERROR)
	{
		mMethod->GenerateErrorResponse(mStatusCode);
		return (1);
	}
	mMethod->ResponseConfigSetup(*mServerPtr, mUri, mSetting);
	// TODO GET /page/www.naver.com HTTP/1.1 으로 요청이 감
	if (mSetting.returnPair.first != 0)
	{
		mStatusCode = mSetting.returnPair.first;
	}
	if (300 <= mStatusCode && mStatusCode < 600)
	{
		mMethod->GenerateRedirectionResponse(mStatusCode, mSetting);
		return (1);
	}
	return (0);
}

void UserData::SetCgiEvent(void)
{
	int fd = mMethod->GetFd();
	UserData* udata = new UserData(fd);

	std::cout << "parent fd: " << fd << std::endl;
	if (mBody == NULL)
	{
		mBody = new std::vector<unsigned char>;
	}
	udata->mBody = mBody;
	udata->mReceived = mReceived;
	udata->mSocketType = CGI_SOCKET;
	udata->mPid = mMethod->GetPid();
	udata->mClientUdata = this;
	WebServer::GetInstance()->ChangeEvent(fd, EVFILT_READ, EV_ADD | EV_DISABLE, udata);
	WebServer::GetInstance()->ChangeEvent(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, udata);
}

void UserData::passBodyToPost(void)
{
	std::string body(mReceived->begin(), mReceived->end());
	std::cout << mMethod->GetType() << Colors::MagentaString("[body]") << body << std::endl;
	mMethod->GenerateResponse(mUri, mSetting, mHeaders, body);
	SetCgiEvent();
	std::cout << body.size() << "|body  contentSize|" << mContentSize << std::endl;
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
	return;
}

static void generateDefaultErrorPage(std::string& response)
{
	response += "Content-type: text/html\r\n"
				"Content-Length: 203\r\n\r\n"
				"<!DOCTYPE HTML>"
				"<html>\n"
				"  <head>\n"
				"    <title>Error page</title>\n"
				"  </head>\n"
				"  <body>\n"
				"    <h1>Sorry. Error occurred</h1>\n"
				"    <p>Server didn't prepare specific error page. What did you do?</p>\n"
				"  </body>\n"
				"</html>";
}

static std::string intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

void UserData::GeneratePostResponse(int status)
{
	// http 블럭에있는 error_page 인자로 받아올것.
	std::string firstLine;
	std::ifstream errorPage;

	if (status == 200)
	{
		firstLine = "HTTP/1.1 200 OK\r\n";
		// insert 버전 200 ok
		mBody->insert(mBody->begin(), firstLine.begin(), firstLine.end());
		return;
	}
	mBody->clear();
	std::string body = "HTTP/1.1 " + WebServer::GetInstance()->GetStatusText(status) + "\r\n";

	std::string errorPageUri = WebServer::GetInstance()->GetErrorPage(status);

	errorPage.open(errorPageUri.c_str(), std::ios::binary);
	if (errorPage.is_open() == false)
	{
		generateDefaultErrorPage(body);
	}
	else
	{
		body += "Content-length: ";
		errorPage.seekg(0, std::ios::end);
		std::streampos fileSize = errorPage.tellg();
		errorPage.seekg(0, std::ios::beg);
		body += intToString(fileSize) + "\r\n\r\n";

		std::stringstream fileContent;
		fileContent << errorPage.rdbuf();
		body += fileContent.str();
		errorPage.close();
	}
	mBody->insert(mBody->begin(), body.begin(), body.end()); // BABO body에 넣어놓고 안 보내줌...
	std::cout << Colors::BoldCyanString("[Body]\n") << body << std::endl;
}

void UserData::ReadRequest(void)
{
	std::string temp;

	if (checkHeaderLength(*mReceived, mHeaderFlag) == false)
	{
		return;
	}

	if (mChunkedFlag == true || mFillBodyFlag == true)
	{
		if (mFillBodyFlag == true && mReceived->size() < mContentSize)
		{
			return;
		}
		if (mChunkedFlag == true && checkChunkedMessageEnd(*mReceived) == false)
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
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ENABLE, this);
	}
	else
	{
		mUri = uriGenerator();
		if (mMethod->GetType() == POST)
		{
			mPostFlag = true;
			if (mReceived->size() < mContentSize)
			{
				if (mSetting.bPostMethod == false)
				{
					mMethod->GenerateErrorResponse(405);
					WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
					WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ENABLE, this);
				}
				else if (mHeaders[TRANSFER_ENCODING] == "chunked")
				{
					if (checkChunkedMessageEnd(*mReceived) == true)
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
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ENABLE, this);
		}
	}
}

int UserData::RecvFromClient(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	if (len <= 0)
	{
		return (len);
	}
	mReceived->insert(mReceived->end(), &mBuf[0], &mBuf[len]);
	// std::string test(&mBuf[0], &mBuf[len]);
	// std::cout << Colors::BoldGreen;
	// for (std::string::iterator it = test.begin(); it != test.end(); it++)
	// {
	// 	std::cout << *it;
	// }
	// std::cout << Colors::Reset << std::endl;
	return (len);
}

int UserData::RecvFromCgi(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	std::cout << mBuf << std::endl;
	if (len <= 0)
	{
		return (len);
	}
	mBody->insert(mBody->end(), &mBuf[0], &mBuf[len]);
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
	if (mPostFlag == true)
	{
		std::cout << "mBody: " << mBody->size() << std::endl;
		if (mBody->size() >= BUFFER_SIZE)
			maxWrite = BUFFER_SIZE;
		else
			maxWrite = mBody->size();
		std::string temp(mBody->begin(), mBody->begin() + maxWrite);
		len = write(fd, mBody->data(), maxWrite); // BABO mBody를 그대로 write하고 있었음.
		if (len < 0)
		{
			Error::Print("send()");
			return (ERROR);
		}
		mBody->erase(mBody->begin(), mBody->begin() + maxWrite);
		if (mMethod->GetResponse().size() <= 0)
		{
			std::cout << Colors::BoldMagenta << "send to client " << fd << "\n" << Colors::Reset << std::endl;
			InitUserData();
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_ENABLE, this);
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_DISABLE, this);
		}
		return (len);
	}
	if (mMethod->GetResponse().size() >= BUFFER_SIZE)
		maxWrite = BUFFER_SIZE;
	else
		maxWrite = mMethod->GetResponse().size();
	len = write(fd, mMethod->GetResponse().c_str(), maxWrite);
	// len = write(1, mMethod->GetResponse().c_str(), maxWrite);
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
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_DISABLE, this);
	}
	return (len);
}

#define MAX_CGI_WRITE_SIZE 500

int UserData::SendToCgi(void)
{
	int len = 0;
	int maxLen;
	std::string temp;

	std::cout << Colors::BoldBlueString("mReceived: ") << mReceived->size() << std::endl;
	// TODO 임시방편
	if (mReceived->size() >= MAX_CGI_WRITE_SIZE)
	{
		maxLen = MAX_CGI_WRITE_SIZE;
	}
	else if (mReceived->size() == 0)
	{
		return (0);
	}
	else
	{
		maxLen = mReceived->size();
	}
	temp.assign(mReceived->begin(), mReceived->begin() + maxLen);
	len = write(1, temp.c_str(), maxLen);
	len = write(mFd, temp.c_str(), maxLen);
	std::cout << Colors::BoldGreenString("CGI 파이프 소켓에 썼음: ") << len << std::endl;
	if (len < 0)
	{
		return (ERROR);
	}
	mReceived->erase(mReceived->begin(), mReceived->begin() + len);
	if (mReceived->size() == 0)
	{
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_ENABLE, this);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_DISABLE, this);
		std::cout << mFd << " CGI write 끄고, read 켜고" << std::endl;
	}
	return (len);
}

int UserData::GetPid(void) const
{
	return (mPid);
}

UserData* UserData::GetClientUdata(void) const
{
	return (mClientUdata);
}
