#pragma once

#include "AMethod.hpp"
#include "UserData.hpp"

class MethodPost : public AMethod
{
public:
	MethodPost(int type);
	~MethodPost(void);

	int GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers);

	void initCgiEnv(std::string httpCgiPath, size_t ContentSize, std::map<int, std::string> Header);
	void execute();
	// void clear();

	const std::map<std::string, std::string>& getEnv() const;
	const pid_t& getCgiPid() const;
	const std::string& getCgiPath() const;
	void sendCgiBody();
	void readCgiResponse(void);

private:
	std::map<std::string, std::string> env;
	char** chEnv;
	char** argv;
	std::string cgiPath;
	pid_t cgiPid;
	int pipeIn[2];
	int pipeOut[2];

	const MethodPost& operator=(const MethodPost& rhs);
	MethodPost(const MethodPost& rhs);
	MethodPost(void);
};