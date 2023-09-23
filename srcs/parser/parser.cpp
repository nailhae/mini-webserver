#include "parser.hpp"
bool LocationParser(LocationBlock& location, std::ifstream &file);
enum BlockType {
	NONE,
	SERVER,
	LOCATION
};

void InitHttpBlock(HttpBlock& http)
{
	http.clientMaxBodySize = 1024;
	http.clientBodyTimeout = 60;
	http.workerConnections = 1024;
}

void InitServerBlock(ServerBlock& server)
{
	server.listenPort = 4242;
}

void InitLocationBlock(LocationBlock& location)
{
	location.bget = false;
	location.bpost = false;
	location.bdeleteMethod = false;
	location.index = "index.html";
}

std::vector<std::string> split(std::string str) {
	std::vector<std::string> vec;
	std::stringstream ss(str);
	std::string temp;

	while (getline(ss, temp, ' ')) {
		vec.push_back(temp);
	}

	return (vec);
}

bool ServerParser(ServerBlock& server, std::ifstream &file) {
	std::stack<BlockType> blockStack;
	blockStack.push(SERVER);
	std::string line;
	while (getline(file, line)) {
		std::istringstream iss(line);
		std::string key;

		if (!(iss >> key)) {continue;}

		if (key == "listen") {
			std::string value;
		if (iss >> value) {
			if (value[value.size() - 1] == ';') {
				value.erase(value.end() - 1);
			}
			else
			{
				return 1;
			}
			server.listenPort = strtol(value.c_str(), NULL, 10);
		} else if (key == "server_name") {
			std::string value;
			if (value[value.size() - 1] == ';') {
				value.erase(value.end() - 1);
			}
			else
			{
				return 1;
			}
			if (iss >> value) {
				server.serverName = value;
			}
		} else if (key == "root") {
			std::string value;
			if (value[value.size() - 1] == ';') {
				value.erase(value.end() - 1); 
			}
			else
			{
				return 1;
			}
			if (iss >> value) {
				server.rootPath = value;
			}
		} else if (key == "location") {
			LocationBlock* location = new LocationBlock;
			std::string value;
			if (iss >> value) {
				location->uri = value;
			}
			if (!(iss >> value) || value.size() != 1 || value[0] != '{')
			{
				return 1;
			}
			if (LocationParser(*location, file)) {
				server.locationList.push_back(location);
			}
			else
			{
				// TODO location parsing에 실패했으므로 동적할당 메모리 해제 후 프로그램 종료 해야함
				return 1;
			}
		} else if (key == "}") {
			blockStack.pop();
			if (blockStack.empty()) {
				return (0);
			}
		}
	}
	return (1);
}

bool LocationParser(LocationBlock& location, std::ifstream &file) {
	std::stack<BlockType> blockStack;
	blockStack.push(LOCATION);
	std::string line;

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string key;

		if (!(iss >> key)) {continue;}

		if (key == "limit_except") {
			std::string value;
			while (iss value && value[value.size() - 1] != ';') {
				if (value == "GET") {
					location.bget = true;
				}
				else if (value == "POST") {
					location.bpost =true;
				}
				else if (vlaue == "DELETE") {
					location.bdeleteMethod = true;
				}
			}
		} else if (key == "autoindex") {
			std::string value;
			if (iss >> value) {
				value.erase(value.end() - 1);
				location.autoindex = (vlaue == "on" ? ture : false);
			}
		} else if (key == "index") {
			std::string value;
			iss >> value;
			value.erase(value.end() - 1);
			location.index = value;
		} else if (key == "alias") {
			std::string value;
			iss >> value;
			value.erase(value.end() - 1);
			location.alias = value;
		} else if (key == "return ") {
			int firstPair = 0;
			std::string secondPair = "";
			iss>>firstPait>>secondPair;
			if (secondPair[secondPair.end() - 1] == ';')
			{
				secondPair.erase(secondPair.end() - 1); 
			}
			else
			{
				return 1;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		} else if (key == "}") {
			blockStack.pop();
			if (blockStack.empty()) {
				return (0);
			}
		} else {
			return 1;
		}
	}
	return 0;
}

bool ParseLine(const std::string& line, std::ifstream file, HttpBlock& http, ServerBlock& server, LocationBlock& location)
{
	// const int success = 1;
	// const int fail = 0;
	std::istringstream iss(line);
	std::string key;

	if (!(iss >> key))
	{
		return (0);
	}
	if (key[0] == '#')
	{
		return (0);
	}
	if (key == "default_type")
	{
		std::string value;
		if (iss >> value)
		{
			if (value[value.end() - 1] == ';')
			{
				value.erase(value.end() - 1);
			}
			else
			{
				return 1;
			}
		}
		http.types["default_type"] = value;
	} else if (key == "client_max_body_size") {
		std::string value;
		if (iss >> value) {
			if (value[value.end() - 1] == ';') {
				value.erase(value.end() - 1); 
			}
			else
			{
				return 1;
			}
			if (value[value.end() - 1] == 'm' || value[value.end() - 1] == 'M') {
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024;
			} else if (value[value.end() - 1] == 'g' || value[value.end() - 1] == 'G') {
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024 * 1024;
			} else if (value[value.end() - 1] == 'k' || value[value.end() - 1] == 'K') {
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
			} else {
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
			}
	} else if (key == "error_pages") {
		std::vector<std::string> result = split(line);
		if (!(result.empty())) {
			std::string errorRoute = result[result.end() - 1];
			if (errorRoute[errorRoute.length() - 1] == ';') {
				errorRoute.erase(errorRoute.length() - 1);
			}
			else
			{
				return 1;
			}
			for (int i = 0; i < result.end() - 1; i++) {
				// TODO value에 값이 없지 않나?
				http.errorPages[result[i]] = errorRoute;
			}
		}
	} else if (key == "server") {
		// } 가 나올때까지
		std::string value;
		if (!(iss >> value) || value.end() != 1 || value[0] != '{')
		{
			return 1;
		}
		if (ServerParser(server, file) == false)
			return (1);
		http.serverList.push_back(&server);
	} else if (key == "location") {
		// TODO location후에  'uri {' 규칙 맞는지 확인하는 코드 추가
		// } 가 나올때까지
		if (LocationParser(location, file) == false)
			return (1);
	}

	if (iss >> key && key[0] != '#')
	{
		return 1;
	}	
	
	return 0;
}

void ParseFile(const std::string& fileName, HttpBlock& http) 
{
	std::ifstream	file(fileName);
	std::string		line;

	ServerBlock* server = new ServerBlock;
	LocationBlock* location = new LocationBlock;

	InitServerBlock(*server);
	InitLocationBlock(*location);
	if(!file.is_open()) {
		std::cout << "Failed to open file" << std::endl;
		return ;
	}

	while (std::getline(file, line)) {
		ParseLine(line, file, http, *server, *location);
		// TODO ParseLine 실패 시 모든 동적할당 메모리 해제 후 conf file 에러라고 알리고 프로그램 종료
		// TODO catch 됐을 때 동적할당 해제.
	}
}