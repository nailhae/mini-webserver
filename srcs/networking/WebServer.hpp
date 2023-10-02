#pragma once

#include "ChangeList.hpp"
#include "dataSet.hpp"

#define CONF_FILE_PATH "../../config/default.conf"

class WebServer
{
public:
	static WebServer* GetInstance();
	static void DeleteInstance();
	const HttpBlock* GetHttp() const;
	int InitServer(void);

private:
	WebServer();
	WebServer(std::string confFile);
	virtual ~WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);

	HttpBlock* parseFileOrNull(const std::string& fileName);
	void deleteHttpBlock(HttpBlock& http);

	static WebServer* mWebServer;
	ChangeList changelist;
	HttpBlock* mHttp;
};
