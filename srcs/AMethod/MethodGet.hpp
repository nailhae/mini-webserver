#pragma once

#include "AMethod.hpp"
#include "UserData.hpp"

class MethodGet : public AMethod
{
public:
	// temp
	MethodGet(int type);
	~MethodGet(void);

	int GenerateResponse(std::string mUri, int autoindex, int& mStatusCode, std::string mResponse,
						 std::map<int, std::string>& mStatusMap, int mFd, UserData& userData,
						 std::map<int, std::string>& mHearders, LocationBlock& mSetting);
	std::string generateErrorResponse(int mStatusCode, std::map<int, std::string>& statusMap);
	std::string generateRedirectionResponse(int mStatusCode, std::map<int, std::string>& statusMap,
											LocationBlock& mSetting);
	std::string generateResponseStatusLine(const std::string& statusValue);
	void generateResponseHeaders(const std::string& extTemp, std::ifstream& requestedFile, int& mStatusCode,
								 std::string mUri, std::string& mResponse, std::map<int, std::string>& mHeaders,
								 std::map<int, std::string>& mStatusMap, LocationBlock& mSetting);
	std::string generateResponseBody(std::ifstream& File);
	std::string setContentType(const std::string& extTemp);
	std::string setContentLength(std::ifstream& _requestedFile);
	std::string setCurrentTime(const char* headerType);
	std::string setModifiedTime(const char* filePath, int& mStatusCode, std::map<int, std::string>& mHeaders);
	std::string setCacheControl(std::map<int, std::string> mHeaders);
	std::string setETag(std::string mUri);

private:
	const MethodGet& operator=(const MethodGet& rhs);
	MethodGet(const MethodGet& rhs);
	MethodGet(void);
};