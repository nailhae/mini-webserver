#include "parser.hpp"

void printLocation(std::vector<LocationBlock *> const &input);

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
	server.rootPath = "";
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

int locationUriErrorCheck(std::vector<LocationBlock *> const &location) {
	for (int i = 0; i < location.size(); i++) {
		if (locationUriErrorCheck(location.at(i)->locationList))
				return 1;
		for (int j = i + 1; j < location.size(); j++) {
			std::cout << location.at(i)->uri << location.at(j)->uri << "\n";
			if (location.at(i)->uri == location.at(j)->uri)
				return 1;
		}
	}
	return 0;
}

int serverListenErrorCheck(std::vector<ServerBlock *> const &server) {
	if (server.empty())
		return 1;
	for (int i = 0; i < server.size(); i++) {
		if (!server.at(i)->listenPort) {
			return(1);
		}
		if (locationUriErrorCheck(server.at(i)->locationList))
				return 1;
		for (int j = i + 1; j < server.size(); j++) {
			if (server.at(i)->listenPort == server.at(j)->listenPort)
				return 1;
		}
	}
	return 0;
}

int parserErrorCheck(HttpBlock &http) {
	//서버블록이 0개일경우
	//서버블록안에 listen이 없을경우, 숫자여야한다, 다른 서버랑 중복이 아니여야한다. 범위는  0~65535
	//로케이션 uri중복인지 아닌지
	if (serverListenErrorCheck(http.serverList)) {
		return 1;
	}
	if (locationUriErrorCheck(http.locationList))
		return 1;
	return 0;
}

int ParseFile(const std::string &fileName, HttpBlock &http)
{
	std::ifstream file(fileName);
	std::string line;

	if (!file.is_open())
	{
		std::cout << "Failed to open file" << std::endl;
		return (1);
	}

	while (std::getline(file, line))
	{
		if (int i = ParseLine(line, file, http)){
			std::cout << "error" << i << '\n';
			return 1;
		}
		// TODO ParseLine 실패 시 모든 동적할당 메모리 해제 후 conf file 에러라고 알리고 프로그램 종료
		// TODO catch 됐을 때 동적할당 해제.
	}
	if (parserErrorCheck(http)){
		std::cout << "errorCheck" << '\n';
		return 1;
	}

	printParserResult(http);
	return (0);
}