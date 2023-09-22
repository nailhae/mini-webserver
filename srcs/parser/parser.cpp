#include "parser.hpp"

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

bool ServerParser(ServerBlock& server, std::ifstream file) {
	std::stack<BlockType> blockStack;
}

bool LocationParser(LocationBlock& location, std::ifstream file) {
	
}

int ParseLine(const std::string& line, std::ifstream file, HttpBlock& http, ServerBlock& server, LocationBlock& location)
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
	if (key == "default_type") {
		std::string value;
		if (iss >> value) {
			if (value[value.size() - 1] == ';') {
				value[value.size() - 1] = '\0';
			}
			else
			{
				return 1;
			}
		}
		http.types["default_type"] = value;
	} else if (key == "client_max_body_size") {
		std::string value;
		char *lpos;
		if (iss >> value) {
			if (value[value.size() - 1] == ';') {
				value[value.size() - 1] = '\0';
			}
			else
			{
				return 1;
			}
			if (value[value.size() - 1] == 'm' || value[value.size() - 1] == 'M') {
				http.clientMaxBodySize = strtol(value.c_str(), &lpos, 10) * 1024;
			} else if (value[value.size() - 1] == 'g' || value[value.size() - 1] == 'G') {
				http.clientMaxBodySize = strtol(value.c_str(), &lpos, 10) * 1024 * 1024;
			} else if (value[value.size() - 1] == 'k' || value[value.size() - 1] == 'K') {
				http.clientMaxBodySize = strtol(value.c_str(), &lpos, 10);
			} else {
				http.clientMaxBodySize = strtol(value.c_str(), &lpos, 10);
			}
	} else if (key == "error_pages") {
		std::vector<std::string> result = split(line);
		int value;
		std::string errorRoute = result.back();
		if (errorRoute[errorRoute.length() - 1] == ';') {
			errorRoute.erase(errorRoute.length() - 1);
		}
		else
		{
			return 1;
		}
		for (int i = 0; i< result.size(); i++) {
			// TODO value에 값이 없지 않나?
			http.errorPages[value] = errorRoute;
		}
	} else if (key == "server") {
		std::string value;
		if (!(iss >> value) || value.size() != 1 || value[0] != '{')
		{
			return 1;
		}
		// } 가 나올때까지
		if (ServerParser(server, file) == false)
			return (1);
	} else if (key == "location") {
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

	ServerBlock server;
	LocationBlock location;

	InitServerBlock(server);
	InitLocationBlock(location);
	if(!file.is_open()) {
		std::cout << "Failed to open file" << std::endl;
		return ;
	}

	while (std::getline(file, line)) {
		ParseLine(line, file, http, server, location);
	}
}