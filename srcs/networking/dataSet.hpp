#pragma once

#include <map>
#include <string>
#include <vector>

class AMethod;
class ChangeList;
class UserData;
class WebServer;
class dataSet;
class Colors;
class Error;
class MultiTree;
class MultiTreeNode;

enum eSetupFlags
{
	URI = 0x10,
	B_GET_SETTING = 0x20,
	B_POST_SETTING = 0x40,
	B_DELETE_SETTING = 0x80,
	B_HEAD_SETTING = 0x100,
	B_AUTOINDEX = 0x200,
	INDEX_PAGE = 0x400,
	LOCATION_ROOT_PATH = 0x800,
	ALIAS = 0x1000,
	RETURN_PAIR_VEC = 0x2000,
};

enum eHeaders
{
	NONE,
	HOST,
	CONNECTION,
	CONTENT_TYPE,
	CONTENT_LENGTH,
	CACHE_CONTROL,
	IF_NONE_MATCH,
	IF_MODIFIED_SINCE,
};

enum eMethod
{
	GET = 0,
	HEAD,
	POST,
	DELETE,
	ERROR = -1
};

enum eBlockType
{
	HTTP,
	SERVER,
	LOCATION
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
	// TODO 이름 통일
	bool bGetMethod;	// default false;
	bool bPostMethod;	// default false;
	bool bDeleteMethod; // default false;
	bool bHeadMethod;	// default false;
	bool autoindex;		// default false
	std::string index;	// default "index.html"
	std::string rootPath;
	std::string alias;
	std::pair<int, std::string> returnPair;
};

struct HttpBlock
{
	std::map<std::string, std::string> types;
	std::map<int, std::string> errorPages;
	std::vector<ServerBlock*> serverList; // default_server = vector<ServerBlock> index 0
	int clientMaxBodySize;				  // default unit kB
	int clientBodyTimeout;				  // default unit sec.
	int workerConnections;
};

struct ResponseSetup
{
	ServerBlock server;
	LocationBlock location;
};
