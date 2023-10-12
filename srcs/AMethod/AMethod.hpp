#pragma once

#include <dirent.h>
#include <map>
#include <sys/stat.h>

#include "dataSet.hpp"

class AMethod
{
public:
	AMethod(int fd, int type);
	virtual ~AMethod();

	virtual int GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders) = 0;
	virtual int GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders,
								 std::string& body) = 0;
	const std::string& GetResponse(void) const;
	void GenerateErrorResponse(int code);
	void GenerateResponseStatusLine(int code);
	void GenerateRedirectionResponse(int code, LocationBlock& mSetting);
	void SetCurrentTime(const char* headerType);
	int SetModifiedTime(const char* filePath, const char* headerTime);
	void SetCacheControl(std::map<int, std::string> mHeaders);
	int SetETag(const std::string& mUri, const std::string& etagHeader);
	int GenerateResponseHeaders(std::ifstream& requestedFile, LocationBlock& mSetting, std::string mUri,
								std::map<int, std::string>& mHeaders);
	void GenerateResponseBody(std::ifstream& File);
	void ResponseConfigSetup(const ServerBlock& server, std::string& uri, LocationBlock& setting);
	int GetType(void) const;
	int GetFd(void) const;
	int GetPid(void) const;
	void SetResponse(const std::string& content);
	void EraseResponse(unsigned int amount);
	void SetSetupFlag(eSetupFlags flag);
	void SetContentLength(std::ifstream& _requestedFile);
	void SetContentType(const std::string& extTemp);
	bool IsFinishSetupFlags(void) const;
	void applySettingLocationBlock(LocationBlock& valueSet, const LocationBlock* valueToSet);

protected:
	std::string mResponse;
	int mFd;
	int mType;
	int mPid;
	int mSetupFlags;

private:
	AMethod();
	AMethod(const AMethod& other);
	AMethod& operator=(const AMethod& other);
};
