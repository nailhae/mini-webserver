#pragma once
#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>

#include "MultiTree.hpp"
#include "MultiTreeNode.hpp"

#define CONF_FILE_PATH "../../config/default.conf"

class MultiTree;
class MultiTreeNode;

enum BlockType
{
	NONE,
	SERVER,
	LOCATION
};

struct LocationBlock;

struct ServerBlock;

struct HttpBlock
{
	std::map<std::string, std::string> types;
	std::map<int, std::string> errorPages;
	std::vector<ServerBlock*> serverList; // default_server = vector<ServerBlock> index 0
	std::vector<MultiTree*> root;
	int clientMaxBodySize; // default unit kB
	int clientBodyTimeout; // default unit sec.
	int workerConnections;
};

struct ServerBlock
{
	int listenPort;
	std::string serverName;
	std::string rootPath;
	std::vector<MultiTree*> root;
};

struct LocationBlock
{
	std::string uri;
	bool bget;			// default false;
	bool bpost;			// default false;
	bool bdeleteMethod; // default false;
	bool bhead; 		// default false;
	bool autoindex;		// default false
	std::string index;	// default "index.html"
	std::string rootPath;
	std::string alias;
	std::pair<int, std::string> returnPair;
};

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

#endif /* WEBSERVER_HPP */
