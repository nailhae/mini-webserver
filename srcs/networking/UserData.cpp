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
	if (mSocketType == CLIENT_SOCKET && mBody != NULL)
	{
		delete mBody;
		mBody = NULL;
	}
	if (mSocketType == CLIENT_SOCKET && mReceived != NULL)
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
		mUri.insert(0, mSetting.rootPath);
	}
	else
	{
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

void UserData::SetClientUdataNULL(void)
{
	mClientUdata = NULL;
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
	std::string lineTemp;

	if (flag == true)
		return (true);
	for (std::vector<unsigned char>::iterator it = received.begin(); it != received.end();)
	{
		pos = std::find(pos, received.end(), '\n');
		if (pos == received.end())
		{
			flag = false;
			return (false);
		}
		lineTemp.assign(it, pos);
		if (lineTemp == "\r" || lineTemp == "")
		{
			flag = true;
			return (true);
		}
		pos += 1;
		it = pos;
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

void UserData::ClearBody(void)
{
	mBody->clear();
}

void UserData::ClearReceived(void)
{
	mReceived->clear();
}

void UserData::SetCgiEvent(void)
{
	int parentSocketFd = mMethod->GetFd();
	int pid = mMethod->GetPid();
	UserData* udataCgi = new UserData(parentSocketFd);

	if (mBody == NULL)
	{
		mBody = new std::vector<unsigned char>;
	}
	udataCgi->mBody = mBody;
	if (udataCgi->mReceived != NULL)
	{
		delete udataCgi->mReceived;
		udataCgi->mReceived = NULL;
	}
	udataCgi->mReceived = mReceived;
	udataCgi->mSocketType = CGI_SOCKET;
	udataCgi->mPid = pid;
	udataCgi->mClientUdata = this;
	mClientUdata = udataCgi;

	WebServer::GetInstance()->ChangeEvent(pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, udataCgi);
	WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_DISABLE, this);
	WebServer::GetInstance()->ChangeEvent(parentSocketFd, EVFILT_READ, EV_ADD | EV_DISABLE, udataCgi);
	WebServer::GetInstance()->ChangeEvent(parentSocketFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, udataCgi);
}

void UserData::passBodyToPost(void)
{
	std::string body(mReceived->begin(), mReceived->end());
	if (mServerPtr->clientMaxBodySize < body.size())
	{
		mMethod->GenerateErrorResponse(413);
		mPostFlag = false;
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ENABLE, this);
	}
	else
	{
		mMethod->GenerateResponse(mUri, mSetting, mHeaders, body);
		SetCgiEvent();
	}
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
	std::string errorPageUri = WebServer::GetInstance()->GetErrorPage(status);
	std::string lineToInsert;
	std::string lineTemp;
	std::ifstream errorPage;
	std::vector<unsigned char>::iterator pos = mBody->begin();

	if (status == 200)
	{
		lineToInsert = "HTTP/1.1 200 OK\r\nContent-Length: ";
		std::cout << mBody->size() << std::endl;
		for (std::vector<unsigned char>::iterator it = mBody->begin(); it != mBody->end();)
		{
			pos = std::find(pos, mBody->end(), '\n');
			if (pos == mBody->end())
				break;
			lineTemp.assign(it, pos);
			if (lineTemp.size() > 0 && *(lineTemp.end() - 1) == '\r')
				lineTemp.erase(lineTemp.size() - 1);
			if (lineTemp.size() == 0)
			{
				lineToInsert += intToString(mBody->size() - (it - mBody->begin()) - 1) + "\r\n";
				break;
			}
			pos += 1;
			it = pos;
		}
		mBody->insert(mBody->begin(), lineToInsert.begin(), lineToInsert.end());
		return;
	}
	mBody->clear();
	std::string body = "HTTP/1.1 " + WebServer::GetInstance()->GetStatusText(status) + "\r\n";
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
	mBody->insert(mBody->begin(), body.begin(), body.end());
}

void UserData::HandlingMethodPost(void)
{
	mPostFlag = true;
	if (mReceived->size() < mContentSize)
	{
		if (mSetting.bPostMethod == false)
		{
			mPostFlag = false;
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
		return;
	}
	else
	{
		passBodyToPost();
	}
	return;
}

void UserData::CheckReceiveAll(void)
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
}

void UserData::ReadRequest(void)
{
	std::string temp;

	if (checkHeaderLength(*mReceived, mHeaderFlag) == false)
	{
		return;
	}
	if (mPostFlag == true)
	{
		CheckReceiveAll();
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
			HandlingMethodPost();
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
	return (len);
}

int UserData::RecvFromCgi(void)
{
	int len;

	len = read(mFd, mBuf, BUFFER_SIZE);
	if (len <= 0)
	{
		return (len);
	}
	mBody->insert(mBody->end(), &mBuf[0], &mBuf[len]);
	return (len);
}

int UserData::SendToClientPostResponse(int fd)
{
	int len;
	int maxWrite;

	if (mBody->size() >= BUFFER_SIZE)
		maxWrite = BUFFER_SIZE;
	else
		maxWrite = mBody->size();
	std::string temp(mBody->begin(), mBody->begin() + maxWrite);
	len = write(fd, mBody->data(), maxWrite);
	if (len < 0)
	{
		Error::Print("send()");
		return (ERROR);
	}
	mBody->erase(mBody->begin(), mBody->begin() + maxWrite);
	if (mBody->size() <= 0)
	{
		std::cout << Colors::BoldMagentaString("send to client ") << fd << "\n" << std::endl;
		InitUserData();
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_ENABLE, this);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_DISABLE, this);
	}
	return (len);
}

int UserData::SendToClient(int fd)
{
	int len;
	int maxWrite;

	if (mPostFlag == true)
	{
		return (SendToClientPostResponse(fd));
	}
	if (mMethod->GetResponse().size() >= BUFFER_SIZE)
		maxWrite = BUFFER_SIZE;
	else
		maxWrite = mMethod->GetResponse().size();
	len = write(fd, mMethod->GetResponse().c_str(), maxWrite);
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

#define MAX_CGI_WRITE_SIZE 1024

int UserData::SendToCgi(void)
{
	int len = 0;
	int maxLen;
	std::string temp;

	std::cout << Colors::BoldBlueString("mReceived: ") << mReceived->size() << std::endl;
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
	len = write(mFd, temp.c_str(), maxLen);
	if (len < 0)
	{
		return (ERROR);
	}
	mReceived->erase(mReceived->begin(), mReceived->begin() + len);
	if (mReceived->size() == 0)
	{
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_READ, EV_ENABLE, this);
		WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_DISABLE, this);
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