#pragma once

#include "ChangeList.hpp"
#include "dataSet.hpp"

#define CONF_FILE_PATH "./config/default.conf"

class WebServer
{
public:
	static WebServer* GetInstance();
	static WebServer* GetInstance(std::string confFile);
	static void DeleteInstance();
	static const std::string& GetStatusText(int code);
	const std::string& GetErrorPage(int code);
	const HttpBlock* GetHttp() const;
	void ChangeEvent(int ident, int nFilter, int nFlags, UserData* udata);
	void WaitForClientConnection(void);

private:
	WebServer();
	WebServer(std::string confFile);
	virtual ~WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);

	void InitKq(void);
	int InitServer(void);
	void HandlingServerSocket(int serverSocket, ServerBlock* serverPtr);
	void HandlingClientSocket(struct kevent& event, UserData* udata);
	void HandlingCGISocket(struct kevent& event, UserData* udata);
	void HandlingTimer(UserData* udata);
	HttpBlock* parseFileOrNull(const std::string& fileName);
	void deleteHttpBlock(HttpBlock& http);
	void acceptClientSocket(int fd, ServerBlock* serverPtr);
	void closeClientSocket(UserData* udata, int fd);
	void ShutdownCgiPid(UserData* udata);
	void closeCgiSocket(UserData* udata, int fd);

	static WebServer* mWebServer;
	ChangeList mChangeList;
	HttpBlock* mHttp;
	int mKq;
	static std::pair<int, std::string> mStatusPair[];
	static std::map<int, std::string> mStatusMap;
};
