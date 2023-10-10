#pragma once

#include "ChangeList.hpp"
#include "dataSet.hpp"

#define CONF_FILE_PATH "../../config/default.conf"

class WebServer
{
public:
	static WebServer* GetInstance();
	static void DeleteInstance();
	static const std::string& GetStatusText(int code);
	const HttpBlock* GetHttp() const;
	void ChangeEvent(int ident, int nFilter, int nFlags, UserData* udata);
	void WaitForClientConnection(void);
	int InitServer(void);

private:
	WebServer();
	WebServer(std::string confFile);
	virtual ~WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);

	HttpBlock* parseFileOrNull(const std::string& fileName);
	void deleteHttpBlock(HttpBlock& http);
	void acceptClientSocket(int fd, ServerBlock* serverPtr);
	void closeClientSocket(UserData* udata, int fd);

	static WebServer* mWebServer;
	ChangeList mChangeList;
	HttpBlock* mHttp;
	static std::pair<int, std::string> mStatusPair[];
	static std::map<int, std::string> mStatusMap;
};
