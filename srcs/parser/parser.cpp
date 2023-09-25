#include "parser.hpp"
int LocationParser(LocationBlock &location, std::ifstream &file);
void printLocation(std::vector<LocationBlock *> const &input);

enum BlockType
{
	NONE,
	SERVER,
	LOCATION
};

void InitHttpBlock(HttpBlock &http)
{
	http.clientMaxBodySize = 1024;
	http.clientBodyTimeout = 60;
	http.workerConnections = 1024;
	http.types.insert(std::make_pair("text/plain", "txt"));
	http.types.insert(std::make_pair("text/html", "html"));
	http.types.insert(std::make_pair("text/css", "css"));
	http.types.insert(std::make_pair("image/png", "png"));
}

void InitServerBlock(ServerBlock &server)
{
	server.listenPort = 4242;
	server.rootPath = "";
	// LocationBlock *location = new LocationBlock;
}

void InitLocationBlock(LocationBlock &location)
{
	location.bget = false;
	location.bpost = false;
	location.bdeleteMethod = false;
	location.autoindex = false;
	location.rootPath = "";
	location.index = "index.html";
}

std::vector<std::string> split(std::string str)
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

int ServerParser(ServerBlock &server, std::ifstream &file)
{
	std::stack<BlockType> blockStack;
	blockStack.push(SERVER);
	std::string line;
	while (getline(file, line))
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
		if (key == "listen")
		{
			std::string value;
			if (iss >> value)
			{
				if (value.back() == ';')
				{
					value.erase(value.size() - 1);
				}
				server.listenPort = strtol(value.c_str(), NULL, 10);
			}
		}
		else if (key == "server_name")
		{
			std::string value;
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 3;
			}
			server.serverName = value;
		}
		else if (key == "root")
		{
			std::string value;
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 3;
			}
			if (!value.empty())
			{
				server.rootPath = value + '/';
			}
			else
			{
				return 3;
			}
		}
		else if (key == "location")
		{
			std::string value;
			if (!(iss >> value) || value[0] != '/')
			{
				return 3;
			}
			LocationBlock *location = new LocationBlock;
			InitLocationBlock(*location);
			if (value != "/")
				location->uri = value + '/';
			if (!(iss >> value) || value.size() != 1 || value[0] != '{')
			{
				return 3;
			}
			if (LocationParser(*location, file) != 0)
			{
				return (3);
			}
			server.locationList.push_back(location);
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return (0);
			}
		}
	}
	return (1);
}

int LocationParser_2(LocationBlock &location, std::ifstream &file)
{
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
				if (value.back() == ';') {
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
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 2;
			}
			if (!(value.empty()))
			{
				location.rootPath = value + '/';
			}
			else
			{
				return 2;
			}
		}
		else if (key == "autoindex")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
				location.autoindex = (value == "on" ? true : false);
			} else {
				return 2;
			}
		}
		else if (key == "index")
		{
			std::string value;

			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			value.erase(value.size() - 1);
			location.index = value;
		}
		else if (key == "alias")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			location.alias = value;
		}
		else if (key == "return")
		{
			int firstPair = 0;
			std::string secondPair = "";
			iss >> firstPair;
			iss >> secondPair;
			if (secondPair.back() == ';')
			{
				secondPair.erase(secondPair.size() - 1);
			}
			else
			{
				return 2;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		} else if (key == "location") {
			std::string value;
			LocationBlock *location_mini = new LocationBlock;
			InitLocationBlock(*location_mini);
			if (!(iss >> value) || value[0] != '/')
			{
				return 1;
			}
			location_mini->uri = value;
			if (!(iss >> value) || value.size() != 1 || value[0] != '{')
			{
				return 1;
			}
			// if (LocationParser(*location_mini, file)){
			// 	std::cout << "loc3 " << '\n';
			// 	return (1);
			// } else {
			// 	location.locationList.push_back(location_mini);
			// }
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return (0);
			}
		}
		else
		{
			return 2;
		}
	}
	return 0;
}

int LocationParser(LocationBlock &location, std::ifstream &file)
{
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
				if (value.back() == ';') {
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
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 2;
			}
			if (!(value.empty()))
			{
				location.rootPath = value + '/';
			}
			else
			{
				return 2;
			}
		}
		else if (key == "autoindex")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
				location.autoindex = (value == "on" ? true : false);
			} else {
				return 2;
			}
		}
		else if (key == "index")
		{
			std::string value;

			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			value.erase(value.size() - 1);
			location.index = value;
		}
		else if (key == "alias")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			location.alias = value;
		}
		else if (key == "return")
		{
			int firstPair = 0;
			std::string secondPair = "";
			iss >> firstPair;
			iss >> secondPair;
			if (secondPair.back() == ';')
			{
				secondPair.erase(secondPair.size() - 1);
			}
			else
			{
				return 2;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		} else if (key == "location") {
			std::string value;
			LocationBlock *location_mini = new LocationBlock;
			InitLocationBlock(*location_mini);
			if (!(iss >> value) || value[0] != '/')
			{
				return 1;
			}
			location_mini->uri = value;
			if (!(iss >> value) || value.size() != 1 || value[0] != '{')
			{
				return 1;
			}
			if (LocationParser_2(*location_mini, file)){
				return (1);
			} else {
				location.locationList.push_back(location_mini);
			}
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return (0);
			}
		}
		else
		{
			return 2;
		}
	}
	return 0;
}

int ParseLine(const std::string &line, std::ifstream &file, HttpBlock &http)
{
	std::istringstream iss(line);
	std::string key;

	if (!(iss >> key) || key[0] == '#')
	{
		return (0);
	}
	if (key == "default_type")
	{
		std::string value;
		if (iss >> value)
		{
			if (value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 1;
			}
			http.types["default_type"] = value;
		}
	}
	else if (key == "client_max_body_size")
	{
		std::string value;
		if (iss >> value)
		{
			if (value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 1;
			}
			if (value.back() == 'm' || value.back() == 'M')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024;
			}
			else if (value.back() == 'g' || value.back() == 'G')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024 * 1024;
			}
			else if (value.back() == 'k' || value.back() == 'K')
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
		if (!(result.empty()))
		{
			std::string errorRoute = result.back();
			if (errorRoute.back() == ';')
			{
				errorRoute.erase(errorRoute.size() - 1);
			}
			else
			{
				return (1);
			}
			for (int i = 0; i < result.size() - 1; i++)
			{
				// TODO value에 값이 없지 않나?
				http.errorPages[strtol(result[i].c_str(), NULL, 10)] = errorRoute;
			}
		}
	}
	else if (key == "server")
	{
		// } 가 나올때까지
		std::string value;
		if (!(iss >> value) || value.size() != 1 || value[0] != '{')
		{
			return 1;
		}
		ServerBlock *server = new ServerBlock;
		InitServerBlock(*server);
		if (ServerParser(*server, file)) {
			return (1);
		}
		http.serverList.push_back(server);
	}
	else if (key == "location")
	{
		std::string value;
		// TODO uri checker가 만들어지면 uri 확인하는 코드도 추가 필요
		if (!(iss >> value) || value[0] != '/')
		{
			return 1;
		}
		LocationBlock *location = new LocationBlock;
		InitLocationBlock(*location);
		location->uri = value + '/';
		if (!(iss >> value) || value.size() != 1 || value[0] != '{')
		{
			return 1;
		}
		// } 가 나올때까지
		if (LocationParser(*location, file)){
			return (1);
		} else {
			http.locationList.push_back(location);
		}
	}

	// if (iss >> key && key[0] != '#')
	// {
	// 	std::cout << key << "123#" << '\n';
	// 	return 1;
	// }

	return 0;
}

using namespace std;

template <typename K, typename V> void print_map(std::map<K, V> &m)
{
	for (typename std::map<K, V>::iterator itr = m.begin(); itr != m.end(); ++itr)
	{
		std::cout << itr->first << " " << itr->second << std::endl;
	}
}

void printLocation(std::vector<LocationBlock *> const &input)
{
	std::cout << input.size() << '\n';
	for (int i = 0; i < input.size(); i++)
	{
		std::cout << "URI: " << input.at(i)->uri << std::endl;
		std::cout << "GET: " << input.at(i)->bget << std::endl;
		std::cout << "POST: " << input.at(i)->bpost << std::endl;
		std::cout << "DELETE: " << input.at(i)->bdeleteMethod << std::endl;
		std::cout << "autoindex: " << input.at(i)->autoindex << std::endl;
		std::cout << "index: " << input.at(i)->index << std::endl;
		std::cout << "alias: " << input.at(i)->alias << std::endl;
		std::cout << "root: " << input.at(i)->rootPath << std::endl;
		std::cout << "return: " << input.at(i)->returnPair.first << " " << input.at(i)->returnPair.second << std::endl;
		std::cout << "--------------------------------" << "\n";
		printLocation(input.at(i)->locationList);
	}
}

void printServer(std::vector<ServerBlock *> const &input)
{
	for (int i = 0; i < input.size(); i++)
	{
		std::cout << "Listen Port: " << input.at(i)->listenPort << std::endl;
		std::cout << "Server Name: " << input.at(i)->serverName << std::endl;
		std::cout << "Root Path: " << input.at(i)->rootPath << std::endl;
		printLocation(input.at(i)->locationList);
	}
}

void printParserResult(HttpBlock &http)
{
	print_map(http.types);
	print_map(http.errorPages);
	std::cout << "client_max_body_size: " << http.clientMaxBodySize << "\n";
	std::cout << "clientBodyTimeout: " << http.clientBodyTimeout << "\n";
	std::cout << "workerConnections: " << http.workerConnections << "\n";
	printLocation(http.locationList);
	printServer(http.serverList);
}

void ParseFile(const std::string &fileName, HttpBlock &http)
{
	std::ifstream file(fileName);
	std::string line;

	if (!file.is_open())
	{
		std::cout << "Failed to open file" << std::endl;
		return;
	}

	while (std::getline(file, line))
	{
		if (int i = ParseLine(line, file, http)){
			std::cout << "error" << i << '\n';
			break;
		}
		// TODO ParseLine 실패 시 모든 동적할당 메모리 해제 후 conf file 에러라고 알리고 프로그램 종료
		// TODO catch 됐을 때 동적할당 해제.
	}

	printParserResult(http);
}