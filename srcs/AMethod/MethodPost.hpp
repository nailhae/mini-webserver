#pragma once

#include "AMethod.hpp"
#include "UserData.hpp"

#define SOCK_PARENT 1
#define SOCK_CHILD 0

class MethodPost : public AMethod
{
public:
	MethodPost(int type);
	~MethodPost(void);

	int GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers,
						 std::string& body);
	void initCgiEnv(std::string httpCgiPath, size_t ContentSize, std::map<int, std::string> Header, std::string body);
	int execute();

	const std::map<std::string, std::string>& getEnv() const;
	const pid_t& getCgiPid() const;
	const std::string& getCgiPath() const;

private:
	std::map<std::string, std::string> env;
	char** chEnv;
	char** argv;
	std::string cgiPath;
	pid_t cgiPid;

	const MethodPost& operator=(const MethodPost& rhs);
	MethodPost(const MethodPost& rhs);
	MethodPost(void);
	int GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers);
};