#include "AMethod.hpp"

#include "MethodPost.hpp"
#include "MultiTree.hpp"
#include "UserData.hpp"

AMethod::AMethod(int fd, int type)
	: mFd(fd)
	, mType(type)
	, mSetupFlags(0)
{
}

AMethod::~AMethod()
{
}

const std::string& AMethod::GetResponse(void) const
{
	return (mResponse);
}

int AMethod::GetFd(void) const
{
	return (mFd);
}

int AMethod::GetPid(void) const
{
	return (mPid);
}

void AMethod::GenerateRedirectionResponse(int code, LocationBlock& mSetting)
{
	GenerateResponseStatusLine(code);
	if (code != 304)
		mResponse += "Location: " + mSetting.returnPair.second + "\r\n";
	mResponse += "\r\n";
}

void AMethod::EraseResponse(unsigned int amount)
{
	mResponse.erase(0, amount);
}

void AMethod::SetCurrentTime(const char* headerType)
{
	time_t rawTime;
	struct tm* timeInfo;
	char buffer[50];

	time(&rawTime);				 // time_t 타입으로 현재 시간을 초 단위로 받아 온다.
	timeInfo = gmtime(&rawTime); // time_t로 받아 온 시간을 로컬 시간으로 변환

	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z",
			 timeInfo); // 시간 정보를 문자열로 변환 후 주어진 버퍼에 저장, 문자열 길이를 반환

	mResponse += headerType + std::string(buffer) + "\r\n";
}

// If-Modified-Since 헤더가 요청에 있을 경우만
int AMethod::SetModifiedTime(const char* filePath, const char* headerTime)
{
	struct stat fileInfo;
	struct tm* timeInfo;
	struct tm headerTimeInfo;
	time_t lastModifiedTime;
	time_t ifModifiedTime;
	char buffer[50];
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
		return (304);
	}
	else
	{
		timeInfo = gmtime(&lastModifiedTime); // time_t로 받아 온 시간을 로컬 시간으로 변환
		strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z",
				 timeInfo); // 시간 정보를 문자열로 변환 후 주어진 버퍼에 저장, 문자열 길이를 반환
		mResponse += "Last Modified: ";
		mResponse += buffer;
		mResponse += "\r\n"; // 리소스 경로
	}
	return (0);
}

void AMethod::SetCacheControl(std::map<int, std::string> mHeaders)
{
	mResponse += "Cache-Control: ";
	mResponse += "max-age=" + mHeaders[CACHE_CONTROL] + "\r\n";
}

int AMethod::SetETag(const std::string& mUri, const std::string& etagHeader)
{
	std::stringstream eTag;
	time_t modifiedTime;

	struct stat fileInfo;
	if (stat(("." + mUri).c_str(), &fileInfo) == 0)
	{
		time(&modifiedTime);
	}
	modifiedTime = fileInfo.st_mtime;
	eTag << mUri << "\\" << modifiedTime;

	if (eTag.str() == etagHeader)
	{
		return (304);
	}
	else
	{
		mResponse += "ETag: " + eTag.str() + "\r\n";
	}
	return (0);
}

int AMethod::GenerateResponseHeaders(std::ifstream& requestedFile, LocationBlock& mSetting, std::string mUri,
									 std::map<int, std::string>& mHeaders)
{
	std::string headers;
	std::string extTemp = mUri.substr(mUri.find_last_of('.') + 1);

	SetContentType(extTemp);
	SetContentLength(requestedFile);
	SetCurrentTime("Date: ");
	// mHeaders
	if (mHeaders.find(CONNECTION) != mHeaders.end())
	{
		mResponse += "Connection: " + mHeaders[CONNECTION] + "\r\n";
	}
	if (mHeaders.find(CACHE_CONTROL) != mHeaders.end())
	{
		SetCacheControl(mHeaders);
	}
	if (mHeaders.find(IF_NONE_MATCH) != mHeaders.end())
	{
		// 요청으로 들어온 ETag 값이 현재의 ETag 값과 동일하다면 "HTTP/1.1 304 Not Modified\r\n", 응답 본문 없음.
		if (SetETag(mUri, mHeaders[IF_NONE_MATCH]) == 304)
		{
			GenerateRedirectionResponse(304, mSetting);
			return (304);
		}
	}
	if (mHeaders.find(IF_MODIFIED_SINCE) != mHeaders.end())
	{
		// mHeaders[IF_MODIFIED_SINCE] value != last-Modified 일때 "HTTP/1.1 304 Not Modified\r\n
		if (SetModifiedTime(mUri.c_str(), mHeaders[IF_MODIFIED_SINCE].c_str()) == 304)
		{
			GenerateRedirectionResponse(304, mSetting);
			return (304);
		}
	}
	mResponse += "\r\n";
	return (0);
}

void AMethod::GenerateResponseStatusLine(int code)
{
	mResponse += "HTTP/1.1 ";
	mResponse += WebServer::GetStatusText(code);
	mResponse += "\r\n";
}

void AMethod::SetContentType(const std::string& extTemp)
{
	mResponse += "Content-type: ";
	if (extTemp == "png")
		mResponse += "image/" + extTemp + "\r\n";
	else if (extTemp == "txt" || extTemp == "html" || extTemp == "css")
		mResponse += "text/" + extTemp + "\r\n";
	else
		mResponse += "application/octet-stream\r\n";
}

static std::string intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

void AMethod::SetContentLength(std::ifstream& _requestedFile)
{
	mResponse += "Content-length: ";
	_requestedFile.seekg(0, std::ios::end);
	std::streampos fileSize = _requestedFile.tellg();
	_requestedFile.seekg(0, std::ios::beg);
	mResponse += intToString(fileSize) + "\r\n";
}

static void generateDefaultErrorPage(std::string& response)
{
	response += "Content-Length: 203\r\n\r\n";
	response += "<!DOCTYPE HTML>"
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

void AMethod::GenerateResponseBody(std::ifstream& File)
{
	std::stringstream fileContent;
	fileContent << File.rdbuf();
	mResponse += fileContent.str();
}

void AMethod::GenerateErrorResponse(int code)
{
	std::string errorUri = WebServer::GetInstance()->GetErrorPage(code);
	std::ifstream errorPage;

	// errorUri = WebServer::GetErrorPage(code)
	GenerateResponseStatusLine(code);
	if (code < 300) // POST with 0 contents
	{
		mResponse += "Content-Length: 0\r\n\r\n";
		return;
	}
	SetContentType("html");
	errorPage.open(errorUri.c_str(), std::ios::binary);
	if (errorPage.is_open() == false)
	{
		generateDefaultErrorPage(mResponse);
	}
	else
	{
		SetContentLength(errorPage);
		mResponse += "\r\n";
		GenerateResponseBody(errorPage);
		errorPage.close();
	}
}

void AMethod::applySettingLocationBlock(LocationBlock& valueSet, const LocationBlock* valueToSet)
{
	valueSet.uri.insert(0, valueToSet->uri);
	if (!(ALIAS & mSetupFlags))
	{
		valueSet.alias = valueToSet->alias;
		mSetupFlags |= ALIAS;
	}
	if (!(B_AUTOINDEX & mSetupFlags) && valueToSet->autoindex != -1)
	{
		valueSet.autoindex = valueToSet->autoindex;
		mSetupFlags |= B_AUTOINDEX;
	}
	if (!((B_DELETE_SETTING & mSetupFlags) || (B_GET_SETTING & mSetupFlags) || (B_POST_SETTING & mSetupFlags) ||
		  (B_HEAD_SETTING & mSetupFlags)) &&
		(valueToSet->bDeleteMethod + valueToSet->bGetMethod + valueToSet->bHeadMethod + valueToSet->bPostMethod > 0))
	{
		valueSet.bDeleteMethod = valueToSet->bDeleteMethod;
		valueSet.bGetMethod = valueToSet->bGetMethod;
		valueSet.bPostMethod = valueToSet->bPostMethod;
		valueSet.bHeadMethod = valueToSet->bHeadMethod;
		mSetupFlags |= B_DELETE_SETTING;
		mSetupFlags |= B_GET_SETTING;
		mSetupFlags |= B_POST_SETTING;
		mSetupFlags |= B_HEAD_SETTING;
	}
	if (!(INDEX_PAGE & mSetupFlags) && valueToSet->index.size() != 0)
	{
		valueSet.index = valueToSet->index;
		mSetupFlags |= INDEX_PAGE;
	}
	if (!(LOCATION_ROOT_PATH & mSetupFlags) && valueToSet->rootPath.size() != 0)
	{
		valueSet.rootPath = valueToSet->rootPath;
		mSetupFlags |= LOCATION_ROOT_PATH;
	}
	if (!(RETURN_PAIR_VEC & mSetupFlags) && valueToSet->returnPair.first != 0)
	{
		valueSet.returnPair = valueToSet->returnPair;
		mSetupFlags |= RETURN_PAIR_VEC;
	}
}

void AMethod::ResponseConfigSetup(const ServerBlock& server, std::string& uri, LocationBlock& setting)
{
	MultiTree* targetTree = NULL;
	MultiTreeNode* targetTreeNode = NULL;
	MultiTreeNode* offset = NULL;
	std::string subString;

	for (std::vector<MultiTree*>::const_iterator it = server.root.begin(); it != server.root.end(); it++)
	{
		subString = uri.substr(0, (*it)->GetRoot()->GetURI().size());
		if ((*it)->GetRoot()->GetURI() == subString)
		{
			if (targetTree == NULL || targetTree->GetRoot()->GetURI().size() < subString.size())
				targetTree = *it;
		}
	}
	if (targetTree == NULL)
		return;
	targetTreeNode = targetTree->searchNodeOrNull(uri);
	if (targetTreeNode == NULL)
		return;
	offset = targetTreeNode;
	while (offset != NULL)
	{
		applySettingLocationBlock(setting, offset->GetLocationBlock());
		offset = offset->ParentNode();
	}
}

int AMethod::GetType() const
{
	return (mType);
}

void AMethod::SetResponse(const std::string& content)
{
	mResponse.append(content);
}
