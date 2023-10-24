#include "WebServer.hpp"

#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "ChangeList.hpp"
#include "Error.hpp"
#include "MultiTree.hpp"
#include "MultiTreeNode.hpp"

#define STATUS_NUM 21

std::pair<int, std::string> WebServer::mStatusPair[] = {std::make_pair(200, "200 OK"),
														std::make_pair(201, "201 Created"),
														std::make_pair(204, "204 No Content"),
														std::make_pair(301, "301 Moved Permanently"),
														std::make_pair(302, "302 Found"),
														std::make_pair(303, "303 See Other"),
														std::make_pair(304, "304 Not Modified"),
														std::make_pair(307, "307 Temporary Redirect"),
														std::make_pair(308, "308 Permanent Redirect"),
														std::make_pair(400, "400 Bad Request"),
														std::make_pair(403, "403 Forbidden"),
														std::make_pair(404, "404 Not Found"),
														std::make_pair(405, "405 Method Not Allowed"),
														std::make_pair(411, "411 Length Required"),
														std::make_pair(413, "413 Payload Too Large"),
														std::make_pair(416, "416 Requested Range Not Satisfiable"),
														std::make_pair(500, "500 Internal Server Error"),
														std::make_pair(501, "501 Not Implemented"),
														std::make_pair(502, "502 Bad Gateway"),
														std::make_pair(504, "504 Gateway Timeout"),
														std::make_pair(505, "505 HTTP Version Not Supported")};

std::map<int, std::string> WebServer::mStatusMap(mStatusPair, mStatusPair + STATUS_NUM);

void printTreeStructure(MultiTreeNode* node, int depth);

WebServer* WebServer::mWebServer = NULL;

WebServer::WebServer()
{
}

WebServer::WebServer(std::string confFile)
	: mHttp(parseFileOrNull(confFile))
{
	if (mHttp == NULL)
	{
		exit(EXIT_FAILURE);
	}
}

WebServer::~WebServer()
{
}

void WebServer::ChangeEvent(int ident, int nFilter, int nFlags, UserData* udata)
{
	mChangeList.ChangeEvent(ident, nFilter, nFlags, udata);
}

WebServer* WebServer::GetInstance()
{
	return mWebServer;
}

WebServer* WebServer::GetInstance(std::string confFile)
{
	if (mWebServer == NULL)
	{
		mWebServer = new WebServer(confFile);
	}
	return mWebServer;
}

void WebServer::DeleteInstance()
{
	mWebServer->deleteHttpBlock(*(mWebServer->mHttp));
	delete mWebServer;
}

void WebServer::deleteHttpBlock(HttpBlock& http)
{
	for (std::vector<ServerBlock*>::iterator it = http.serverList.begin(); it != http.serverList.end(); it++)
	{
		for (std::vector<MultiTree*>::iterator treeIt = (*it)->root.begin(); treeIt != (*it)->root.end(); treeIt++)
		{
			delete (*treeIt);
		}
		delete (*it);
	}
	delete &http;
}

const HttpBlock* WebServer::GetHttp() const
{
	return mWebServer->mHttp;
}

const std::string& WebServer::GetStatusText(int code)
{
	return (mStatusMap[code]);
}

const std::string& WebServer::GetErrorPage(int code)
{
	return (mHttp->errorPages[code]);
}
