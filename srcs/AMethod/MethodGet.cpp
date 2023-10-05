#include "MethodGet.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include "ChangeList.hpp"
#include "Colors.hpp"
#include "Error.hpp"
#include "WebServer.hpp"

MethodGet::MethodGet(int type)
	: AMethod(type)
{
}

// temp
MethodGet::~MethodGet(void)
{
}

std::string intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string MethodGet::generateResponseStatusLine(const std::string& statusValue)
{
	// std::string response = "HTTP/1.1 " + statusValue + "\r\n";
	return "HTTP/1.1 " + statusValue + "\r\n";
}

std::string MethodGet::generateErrorResponse(int mStatusCode, std::map<int, std::string>& statusMap)
{
	// http 블럭에있는 error_page 인자로 받아올것.
	std::string response;
	std::string errorUri;
	std::ifstream errorFile;

	errorUri = "./error/" + intToString(mStatusCode) + ".html";

	response = generateResponseStatusLine(statusMap[mStatusCode]);
	response += setContentType("html");
	errorFile.open(errorUri.c_str(), std::ios::binary);
	response += setContentLength(errorFile);
	response += "\r\n";
	response += generateResponseBody(errorFile); // error_paget 본문
	errorFile.close();
	return response;
}

std::string MethodGet::generateRedirectionResponse(int mStatusCode, std::map<int, std::string>& statusMap,
												   LocationBlock& mSetting)
{
	// 304일때 따로 처리 해주기.
	std::string response;
	response = generateResponseStatusLine(statusMap[mStatusCode]);
	if (mStatusCode != 304)
		response += "Location: " + mSetting.returnPair.second + "\r\n"; // 리디렉션할 URI
	response += "\r\n";
	return response;
}

std::string MethodGet::setContentType(const std::string& extTemp)
{
	std::string response = "Content-type: ";
	if (extTemp == "png")
		response += "image/" + extTemp + "\r\n";
	else
		response += "text/" + extTemp + "\r\n";
	return response;
}

std::string MethodGet::setContentLength(std::ifstream& _requestedFile)
{
	std::string response = "Content-length: ";

	_requestedFile.seekg(0, std::ios::end);
	std::streampos fileSize = _requestedFile.tellg();
	_requestedFile.seekg(0, std::ios::beg);
	response += intToString(fileSize) + "\r\n";

	return response;
}

std::string MethodGet::setCurrentTime(const char* headerType)
{
	time_t rawTime;
	struct tm* timeInfo;
	char buffer[50];

	time(&rawTime);				 // time_t 타입으로 현재 시간을 초 단위로 받아 온다.
	timeInfo = gmtime(&rawTime); // time_t로 받아 온 시간을 로컬 시간으로 변환

	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z",
			 timeInfo); // 시간 정보를 문자열로 변환 후 주어진 버퍼에 저장, 문자열 길이를 반환

	return headerType + std::string(buffer) + "\r\n";
}

// If-Modified-Since 헤더가 요청에 있을 경우만
std::string MethodGet::setModifiedTime(const char* filePath, int& mStatusCode, std::map<int, std::string>& mHeaders)
{
	struct stat fileInfo;
	struct tm* timeInfo;
	struct tm headerTimeInfo;
	time_t lastModifiedTime;
	time_t ifModifiedTime;
	char buffer[50];
	const char* headerTime = (mHeaders[IF_MODIFIED_SINCE].c_str());
	strptime(headerTime, "%a, %d %b %Y %H:%M:%S %Z", &headerTimeInfo);
	ifModifiedTime = mktime(&headerTimeInfo);

	if (stat(filePath, &fileInfo) == 0)
	{
		lastModifiedTime = fileInfo.st_mtime;
	}
	else // Failed to get file information
	{
		time(&lastModifiedTime);
	}
	if (ifModifiedTime >= lastModifiedTime)
	{
		mStatusCode = 304;
	}
	else
	{
		timeInfo = gmtime(&lastModifiedTime); // time_t로 받아 온 시간을 로컬 시간으로 변환
		strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z",
				 timeInfo); // 시간 정보를 문자열로 변환 후 주어진 버퍼에 저장, 문자열 길이를 반환
	}
	return std::string(buffer);
}

std::string MethodGet::setCacheControl(std::map<int, std::string> mHeaders)
{
	std::string response = "Cache-Control: ";
	response += "max-age=" + mHeaders[CACHE_CONTROL] + "\r\n";
	return response;
}

std::string MethodGet::setETag(std::string mUri)
{
	std::stringstream eTag;
	time_t modifiedTime;

	struct stat fileInfo;
	if (stat(("." + mUri).c_str(), &fileInfo) == 0)
	{
		// 정보 읽어올 수 없을 경우 어떻게 하지?
		time(&modifiedTime);
	}
	modifiedTime = fileInfo.st_mtime;
	// 파일 정보에서 수정된 시간으로 etag 설정
	eTag << mUri << "\\" << modifiedTime;

	return eTag.str();
}

void MethodGet::generateResponseHeaders(const std::string& extTemp, std::ifstream& requestedFile, int& mStatusCode,
										std::string mUri, std::string& mResponse, std::map<int, std::string>& mHeaders,
										std::map<int, std::string>& mStatusMap, LocationBlock& mSetting)
{
	std::string headers;
	mResponse += setContentType(extTemp);
	mResponse += setContentLength(requestedFile);
	mResponse += setCurrentTime("Date: ");
	// mHeaders
	if (mHeaders.find(CONNECTION) != mHeaders.end())
	{
		mResponse += "Connection: " + mHeaders[CONNECTION] + "\r\n";
	}
	if (mHeaders.find(CACHE_CONTROL) != mHeaders.end())
	{
		mResponse += setCacheControl(mHeaders);
	}
	if (mHeaders.find(IF_NONE_MATCH) != mHeaders.end())
	{
		// 요청으로 들어온 ETag 값이 현재의 ETag 값과 동일하다면 "HTTP/1.1 304 Not Modified\r\n", 응답 본문 없음.
		std::string eTag = setETag(mUri);
		if (eTag == mHeaders[IF_NONE_MATCH])
		{
			mStatusCode = 304;
			mResponse = generateRedirectionResponse(mStatusCode, mStatusMap, mSetting);
			return;
		}
		else
			mResponse += "ETag: " + eTag + "\r\n";
	}
	if (mHeaders.find(IF_MODIFIED_SINCE) != mHeaders.end())
	{
		// mHeaders[IF_MODIFIED_SINCE] value != last-Modified 일때 "HTTP/1.1 304 Not Modified\r\n
		std::string modifiedTime = setModifiedTime(("." + mUri).c_str(), mStatusCode, mHeaders);
		if (mStatusCode == 304)
		{
			mResponse = generateRedirectionResponse(mStatusCode, mStatusMap, mSetting);
			return;
		}
		else
		{
			mResponse += "Last Modified: " + modifiedTime + "\r\n"; // 리소스 경로
		}
	}
	mResponse += "\r\n";
}

std::string MethodGet::generateResponseBody(std::ifstream& File)
{
	std::stringstream fileContent;
	fileContent << File.rdbuf();
	return fileContent.str();
}

int AutoIndexResponse(std::string mUri, int autoindex, int mFd, int& mStatusCode, std::string mResponse,
					  UserData& userData)
{
	if (*(mUri.end() - 1) == '/') // 폴더에 대한 요청은 무조건 autoindex로
	{
		int result = 0;
		std::cout << "autoindex == " << autoindex << std::endl;
		if (autoindex == true)
		{
			result = userData.loadFolderContent();
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
			mResponse += "text/html\r\n";
			mResponse += "\r\nContent-length: ";
			// mResponse += intToString(mBody.size());
			mResponse += "\r\n\r\n";
			// mResponse += mBody;
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, &userData);
		}
	}
	return (0);
}

int MethodGet::GenerateResponse(std::string mUri, int autoindex, int& mStatusCode, std::string mResponse,
								std::map<int, std::string>& mStatusMap, int mFd, UserData& userData,
								std::map<int, std::string>& mHeaders, LocationBlock& mSetting)
{
	std::ifstream requestedFile;
	std::string extTemp;

	if (*(mUri.end() - 1) == '/')
	{
		AutoIndexResponse(mUri, autoindex, mFd, mStatusCode, mResponse, userData);
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
			mStatusCode = 200;
			mResponse = generateResponseStatusLine(mStatusMap[mStatusCode]);
			generateResponseHeaders(extTemp, requestedFile, mStatusCode, mUri, mResponse, mHeaders, mStatusMap,
									mSetting);
			if (mStatusCode != 304)
			{
				mResponse += generateResponseBody(requestedFile);
			}
			requestedFile.close();
			WebServer::GetInstance()->ChangeEvent(mFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, &userData);
		}
	}
	return (0);
}