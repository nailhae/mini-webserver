#pragma once

#include "AMethod.hpp"

class MethodDelete : public AMethod
{
public:
	MethodDelete(int type);
	~MethodDelete(void);

	int GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders);

private:
	const MethodDelete& operator=(const MethodDelete& rhs);
	MethodDelete(const MethodDelete& rhs);
	MethodDelete(void);
	int GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders,
						 std::string& body);
};