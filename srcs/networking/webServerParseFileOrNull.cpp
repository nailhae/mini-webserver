#include "Error.hpp"
#include "MultiTree.hpp"
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
		Error::Print("open file");
		return NULL;
	}

	while (std::getline(file, line))
	{
		line = removeComment(line);
		if (int i = parseLine(line, file, *http))
		{
			Error::Print("failed parsing");
			return NULL;
		}
	}
	if (parserErrorCheck(*http))
	{
		return NULL;
	}

	return http;
}

static int parserErrorCheck(HttpBlock& http)
{
	const int success = 0;
	const int error = 1;
	int portTemp;

	for (std::vector<ServerBlock*>::iterator it = http.serverList.begin(); it != http.serverList.end(); it++)
	{
		portTemp = (*it)->listenPort;
		for (std::vector<ServerBlock*>::iterator nextIt = it + 1; nextIt != http.serverList.end(); nextIt++)
		{
			if (portTemp == (*nextIt)->listenPort)
			{
				Error::Print("duplicate server port");
				return error;
			}
		}
		for (std::vector<MultiTree*>::iterator treeIt = (*it)->root.begin(); treeIt != (*it)->root.end(); treeIt++)
		{
			if ((*treeIt)->CheckDuplicateError() == false)
			{
				Error::Print("duplicate location uri");
				return error;
			}
		}
	}

	return success;
}

static void initHttpBlock(HttpBlock& http)
{
	http.clientBodyTimeout = 60;
	http.workerConnections = 1024;
	http.types.insert(std::make_pair("text/plain", "txt"));
	http.types.insert(std::make_pair("text/html", "html"));
	http.types.insert(std::make_pair("text/css", "css"));
	http.types.insert(std::make_pair("image/png", "png"));
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
				Error::Print("default_type value error");
				return error;
			}
			http.types["default_type"] = value;
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
				Error::Print("error_page value error");
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
			Error::Print("server value error");
			return error;
		}
		ServerBlock* server = new ServerBlock;
		http.serverList.push_back(server);
		initServerBlock(*server);
		if (serverParser(*server, file))
		{
			Error::Print("server block compose error");
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
		if (http.serverList.size() == 0)
		{
			Error::Print("A Location Block can only be in a Server Block");
			return error;
		}
		LocationBlock* location = new LocationBlock;
		initLocationBlock(*location);
		MultiTreeNode* rootNode = new MultiTreeNode(location);
		MultiTree* tree = new MultiTree(*rootNode);

		ServerBlock* currentServer = http.serverList.at(http.serverList.size() - 1);
		currentServer->root.push_back(tree);

		if (value.at(value.size() - 1) == '/')
		{
			value.erase(value.size() - 1);
		}
		location->uri = value;
		if (!(iss >> value) || value != "{")
		{
			Error::Print("location block compose error");
			return error;
		}
		if (locationParser(*location, file, *tree, location->uri))
		{
			Error::Print("location block compose error");
			return error;
		}
	}
	else
	{
		Error::Print("unknown type error");
		return error;
	}

	if (iss >> key && key[0] != '#')
	{
		Error::Print("parse error");
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

static void initServerBlock(ServerBlock& server)
{
	server.rootPath = "";
	server.clientMaxBodySize = 1024;
}

static void initLocationBlock(LocationBlock& location)
{
	location.bGetMethod = false;
	location.bPostMethod = false;
	location.bDeleteMethod = false;
	location.autoindex = -1;
	location.rootPath = "";
	location.index = "index.html";
	location.returnPair.first = 0;
}

static int serverParser(ServerBlock& server, std::ifstream& file)
{
	const int success = 0;
	const int error = 1;
	std::stack<eBlockType> blockStack;
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
					server.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024;
				}
				else if (value[value.size() - 1] == 'g' || value[value.size() - 1] == 'G')
				{
					server.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024 * 1024;
				}
				else if (value[value.size() - 1] == 'k' || value[value.size() - 1] == 'K')
				{
					server.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
				}
				else
				{
					server.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
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
			if (!value.empty())
			{
				server.rootPath = value;
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
	std::stack<eBlockType> blockStack;
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
					location.bGetMethod = true;
				}
				else if (value == "POST")
				{
					location.bPostMethod = true;
				}
				else if (value == "HEAD")
				{
					location.bHeadMethod = true;
				}
				else if (value == "DELETE")
				{
					location.bDeleteMethod = true;
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
				location.rootPath = value;
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
