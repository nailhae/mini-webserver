#pragma once

#include <map>
#include <stack>
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
	URI = 0x0001,
	B_GET_SETTING = 0x0002,
	B_POST_SETTING = 0x0004,
	B_DELETE_SETTING = 0x0008,
	B_HEAD_SETTING = 0x0010,
	B_AUTOINDEX = 0x0020,
	INDEX_PAGE = 0x0040,
	LOCATION_ROOT_PATH = 0x0080,
	ALIAS = 0x0100,
	RETURN_PAIR_VEC = 0x0200,
};

enum eHeaders
{
	NONE,
	HOST,
	CONNECTION,
	CONTENT_TYPE,
	CONTENT_LENGTH,
	TRANSFER_ENCODING,
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

enum eIdentifierType
{
	SERVER_SOCKET,
	CLIENT_SOCKET,
	CGI_SOCKET,
	CGI_PID,
};

struct ServerBlock
{
	std::string serverName;
	std::string rootPath;
	std::vector<MultiTree*> root;
	size_t clientMaxBodySize;
	int listenPort;
};

struct LocationBlock
{
	std::string uri;
	bool bGetMethod;
	bool bPostMethod;
	bool bDeleteMethod;
	bool bHeadMethod;
	int autoindex;
	std::string index;
	std::string rootPath;
	std::string alias;
	std::pair<int, std::string> returnPair;
};

struct HttpBlock
{
	std::map<std::string, std::string> types;
	std::map<int, std::string> errorPages;
	std::vector<ServerBlock*> serverList;
	int clientBodyTimeout;
	int workerConnections;
};
