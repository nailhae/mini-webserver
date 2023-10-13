#pragma once

#include "AMethod.hpp"
#include "UserData.hpp"

class MethodHead : public AMethod
{
public:
	MethodHead(int fd);
	~MethodHead(void);

	int GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders);
	int AutoIndexResponse(std::string& mUri);

private:
	const MethodHead& operator=(const MethodHead& rhs);
	MethodHead(const MethodHead& rhs);
	MethodHead(void);
	int GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders,
						 std::string& body);
};
