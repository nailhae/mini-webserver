#include "WebServer.hpp"

static void initHttpBlock(HttpBlock& http);
static void initServerBlock(ServerBlock& server);
static void initLocationBlock(LocationBlock& location);
static std::string removeComment(const std::string& line);
static int parserErrorCheck(HttpBlock& http);
static int parseLine(const std::string& line, std::ifstream& file, HttpBlock& http);
static std::vector<std::string> split(std::string str);
static int serverParser(ServerBlock& server, std::ifstream& file);
static int locationParser(LocationBlock& location, std::ifstream& file, MultiTree& root, std::string uri);
void printTreeStructure(MultiTreeNode* node, int depth);

WebServer* WebServer::mWebServer = NULL;

WebServer::WebServer()
{
}

WebServer::WebServer(std::string confFile)
	: mHttp(parseFileOrNull(confFile))
{
	const int error = 1;

	if (mHttp == NULL)
	{
		std::cout << " Failed to parse .conf file" << std::endl;
		exit(error);
	}
}

WebServer::~WebServer()
{
}

WebServer::WebServer(const WebServer& other)
{
	(void)other;
}

WebServer& WebServer::operator=(const WebServer& other)
{
	if (this == &other)
	{
		return *this;
	}

	return *this;
}

WebServer* WebServer::GetInstance()
{
	if (mWebServer == NULL)
	{
		mWebServer = new WebServer(CONF_FILE_PATH);
	}
	return mWebServer;
}

void WebServer::DeleteInstance()
{
	mWebServer->deleteHttpBlock(*(mWebServer->mHttp));
	delete mWebServer;
}

HttpBlock* WebServer::parseFileOrNull(const std::string& fileName)
{
	HttpBlock* http = new HttpBlock;

	if (http == NULL)
		return NULL;

	initHttpBlock(*http);

	std::ifstream file(fileName.c_str());
	std::string line;

	if (!file.is_open())
	{
		std::cout << "Failed to open file" << std::endl;
		return NULL;
	}

	while (std::getline(file, line))
	{
		line = removeComment(line);
		if (int i = parseLine(line, file, *http))
		{
			std::cout << "error " << i << '\n';
			return NULL;
		}
	}
	if (parserErrorCheck(*http))
	{
		std::cout << "errorCheck" << '\n';
		return NULL;
	}

	return http;
}

void WebServer::deleteHttpBlock(HttpBlock& http)
{
	for (std::vector<MultiTree*>::iterator it = http.root.begin(); it != http.root.end(); it++)
	{
		delete (*it);
	}
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

static void initHttpBlock(HttpBlock& http)
{
	http.clientMaxBodySize = 1024;
	http.clientBodyTimeout = 60;
	http.workerConnections = 1024;
	http.types.insert(std::make_pair("text/plain", "txt"));
	http.types.insert(std::make_pair("text/html", "html"));
	http.types.insert(std::make_pair("text/css", "css"));
	http.types.insert(std::make_pair("image/png", "png"));
}

static void initServerBlock(ServerBlock& server)
{
	server.rootPath = "";
}

static void initLocationBlock(LocationBlock& location)
{
	location.bget = false;
	location.bpost = false;
	location.bdeleteMethod = false;
	location.autoindex = false;
	location.rootPath = "";
	location.index = "index.html";
}

static std::string removeComment(const std::string& line)
{
	std::size_t pos = line.find('#');
	if (pos != std::string::npos)
	{
		return line.substr(0, pos);
	}
	return line;
}

static int parseLine(const std::string& line, std::ifstream& file, HttpBlock& http)
{
	const int success = 0;
	const int error = 1;
	std::istringstream iss(line);
	std::string key;

	if (!(iss >> key) || key[0] == '#')
	{
		return success;
	}

	if (key == "default_type")
	{
		std::string value;
		if (iss >> value)
		{
			if (value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			http.types["default_type"] = value;
		}
	}
	else if (key == "client_max_body_size")
	{
		std::string value;
		if (iss >> value)
		{
			if (value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			if (value[value.size() - 1] == 'm' || value[value.size() - 1] == 'M')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024;
			}
			else if (value[value.size() - 1] == 'g' || value[value.size() - 1] == 'G')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024 * 1024;
			}
			else if (value[value.size() - 1] == 'k' || value[value.size() - 1] == 'K')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
			}
			else
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
			}
		}
	}
	else if (key == "error_page")
	{
		std::vector<std::string> result = split(line);
		std::string value;
		if (!(result.empty()))
		{
			std::string errorRoute = result.at(result.size() - 1);
			if (errorRoute.at(errorRoute.size() - 1) == ';')
			{
				errorRoute.erase(errorRoute.size() - 1);
			}
			else
			{
				return error;
			}
			for (size_t i = 0; i < result.size() - 1; i++)
			{
				http.errorPages[strtol(result[i].c_str(), NULL, 10)] = errorRoute;
				iss >> value;
			}
			iss >> value;
		}
	}
	else if (key == "server")
	{
		std::string value;
		if (!(iss >> value) || value != "{")
		{
			return error;
		}
		ServerBlock* server = new ServerBlock;
		http.serverList.push_back(server);
		initServerBlock(*server);
		if (serverParser(*server, file))
		{
			return error;
		}
	}
	else if (key == "location")
	{
		std::string value;
		// TODO uri checker가 만들어지면 uri 확인하는 코드도 추가 필요
		if (!(iss >> value) || value[0] != '/')
		{
			return error;
		}
		LocationBlock* location = new LocationBlock;
		initLocationBlock(*location);
		MultiTreeNode* root = new MultiTreeNode(location);
		MultiTree* tree = new MultiTree(*root);
		http.root.push_back(tree);
		if (value.at(value.size() - 1) == '/')
		{
			value.erase(value.size() - 1);
		}
		location->uri = value;
		if (!(iss >> value) || value != "{")
		{
			return error;
		}
		if (locationParser(*location, file, *http.root.at(http.root.size() - 1), location->uri))
		{
			return error;
		}
	}
	else
	{
		return error;
	}

	if (iss >> key && key[0] != '#')
	{
		return error;
	}

	return success;
}

static std::vector<std::string> split(std::string str)
{
	std::vector<std::string> vec;
	std::stringstream ss(str);
	std::string temp;
	ss >> temp;
	while (getline(ss, temp, ' '))
	{
		if (!temp.empty())
		{
			vec.push_back(temp);
		}
	}

	return (vec);
}

static int parserErrorCheck(HttpBlock& http)
{
	const int success = 0;
	const int error = 1;
	// 서버블록이 0개일경우
	// 서버블록안에 listen이 없을경우, 숫자여야한다, 다른 서버랑 중복이 아니여야한다. 범위는  0~65535
	// 로케이션 uri중복인지 아닌지
	for (std::vector<MultiTree*>::iterator it = http.root.begin(); it != http.root.end(); it++)
	{
		if ((*it)->CheckDuplicateError() == false)
			return error;
	}

	return success;
}

static int serverParser(ServerBlock& server, std::ifstream& file)
{
	const int success = 0;
	const int error = 1;
	std::stack<BlockType> blockStack;
	blockStack.push(SERVER);
	std::string line;
	while (getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
		char* pos;
		if (!(iss >> key))
		{
			continue;
		}
		if (key[0] == '#')
		{
			continue;
		}
		if (key == "listen")
		{
			std::string value;
			if (iss >> value)
			{
				if (value.at(value.size() - 1) == ';')
				{
					value.erase(value.size() - 1);
				}
				if (value.empty())
					return error;
				server.listenPort = strtol(value.c_str(), &pos, 10);
				if (*pos)
					return error;
				if (server.listenPort < 0 && 65535 > server.listenPort)
					return error;
			}
			if ((iss >> value) && value != "default_server;")
			{
				return error;
			}
		}
		else if (key == "server_name")
		{
			std::string value;
			if ((iss >> value) && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			server.serverName = value;
		}
		else if (key == "root")
		{
			std::string value;
			if ((iss >> value) && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			if (!value.empty())
			{
				server.rootPath = value + '/';
			}
			else
			{
				return error;
			}
		}
		else if (key == "location")
		{
			std::string value;
			if (!(iss >> value) || value[0] != '/')
			{
				return error;
			}
			LocationBlock* location = new LocationBlock;
			initLocationBlock(*location);
			MultiTreeNode* temp = new MultiTreeNode(location);
			MultiTree* tree = new MultiTree(*temp);
			server.root.push_back(tree);
			if (value.at(value.size() - 1) == '/')
			{
				value.erase(value.size() - 1);
			}
			location->uri = value;
			if (!(iss >> value) || value != "{")
			{
				return error;
			}
			if (locationParser(*location, file, *server.root.at(server.root.size() - 1), location->uri) != 0)
			{
				return error;
			}
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return success;
			}
		}
	}
	return error;
}

static int locationParser(LocationBlock& location, std::ifstream& file, MultiTree& root, std::string uri)
{
	const int success = 0;
	const int error = 1;
	std::stack<BlockType> blockStack;
	blockStack.push(LOCATION);

	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
		if (!(iss >> key))
		{
			continue;
		}
		if (key[0] == '#')
		{
			continue;
		}
		if (key == "limit_except")
		{
			std::string value;
			while ((iss >> value))
			{
				if (value.at(value.size() - 1) == ';')
				{
					value.erase(value.size() - 1);
				}
				if (value == "GET")
				{
					location.bget = true;
				}
				else if (value == "POST")
				{
					location.bpost = true;
				}
				else if (value == "DELETE")
				{
					location.bdeleteMethod = true;
				}
			}
		}
		else if (key == "root")
		{
			std::string value;
			if ((iss >> value) && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			if (!(value.empty()))
			{
				location.rootPath = value + '/';
			}
			else
			{
				return error;
			}
		}
		else if (key == "autoindex")
		{
			std::string value;
			if (iss >> value && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
				location.autoindex = (value == "on" ? true : false);
			}
			else
			{
				return error;
			}
		}
		else if (key == "index")
		{
			std::string value;

			if (iss >> value && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			value.erase(value.size() - 1);
			location.index = value;
		}
		else if (key == "alias")
		{
			std::string value;
			if (iss >> value && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return error;
			}
			location.alias = value;
		}
		else if (key == "return")
		{
			int firstPair = 0;
			std::string secondPair = "";
			iss >> firstPair;
			iss >> secondPair;
			if (secondPair.at(secondPair.size() - 1) == ';')
			{
				secondPair.erase(secondPair.size() - 1);
			}
			else
			{
				return error;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		}
		else if (key == "location")
		{
			std::string value;
			LocationBlock* locationChild = new LocationBlock;
			initLocationBlock(*locationChild);
			if (!(iss >> value) || value[0] != '/')
			{
				delete locationChild;
				return 1;
			}
			if (value.at(value.size() - 1) == '/')
			{
				value.erase(value.size() - 1);
			}
			locationChild->uri = value;
			if (!(iss >> value) || value != "{")
			{
				delete locationChild;
				return 1;
			}
			root.searchNodeOrNull(uri)->AddChildNode(locationChild);
			if (locationParser(*locationChild, file, root, uri + locationChild->uri))
			{
				return error;
			}
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return success;
			}
		}
		else
		{
			return error;
		}
	}
	return success;
}

void printTreeStructure(MultiTreeNode* node, int depth = 0)
{
	if (node == nullptr)
	{
		return;
	}
	// 현재 노드 정보 출력
	const LocationBlock* data = node->GetLocationBlock();
	std::cout << std::string(depth, ' ') << "URI: " << data->uri << std::endl;
	// 자식 노드 순회 및 출력
	for (std::vector<MultiTreeNode*>::const_iterator it = node->GetChildren().begin(); it != node->GetChildren().end();
		 it++)
	{
		printTreeStructure((*it), depth + 2); // 들여쓰기 레벨 조절
	}
}