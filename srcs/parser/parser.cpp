#include "parser.hpp"
#include "../util/MultiTree.hpp"
#include "../util/MultiTreeNode.hpp"

void printLocation(std::vector<LocationBlock *> const &input);

std::string RemoveComment(const std::string& line) {
	std::size_t pos = line.find('#');
    if (pos != std::string::npos) {
        return line.substr(0, pos);
    }
    return line;
}

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

void printServer(std::vector<ServerBlock *>& input)
{
	for (std::vector<ServerBlock *>::iterator it = input.begin(); it != input.end(); it++)
	{
		std::cout << "Listen Port: " << (*it)->listenPort << std::endl;
		std::cout << "Server Name: " << (*it)->serverName << std::endl;
		std::cout << "Root Path: " << (*it)->rootPath << std::endl;
		for (std::vector<MultiTree *>::iterator treeIt = (*it)->root.begin(); treeIt != (*it)->root.end(); treeIt++)
			(*treeIt)->PrintEveryNodes();
	}
}

void printParserResult(HttpBlock &http)
{
	print_map(http.types);
	print_map(http.errorPages);
	std::cout << "client_max_body_size: " << http.clientMaxBodySize << "\n";
	std::cout << "clientBodyTimeout: " << http.clientBodyTimeout << "\n";
	std::cout << "workerConnections: " << http.workerConnections << "\n";
	for (std::vector<MultiTree *>::iterator it = http.root.begin(); it != http.root.end(); it++)
		(*it)->PrintEveryNodes();
	printServer(http.serverList);
}

// int locationUriErrorCheck(std::vector<LocationBlock*> const &location) {
// 	for (size_t i = 0; i < location.size(); i++) 
// 	{
// 		std::cout << location.at(i)->uri << "\n";
// 		if (locationUriErrorCheck(location.at(i)->locationList))
// 				return 1;
// 		for (size_t j = i + 1; j < location.size(); j++) 
// 		{
// 			std::cout << location.at(i)->uri << location.at(j)->uri << "\n";
// 			if (location.at(i)->uri == location.at(j)->uri)
// 				return 1;
// 		}
// 	}
// 	return 0;
// }

int serverListenErrorCheck(std::vector<ServerBlock *> const &server) {
	if (server.empty())
		return 1;
	for (std::vector<ServerBlock*>::const_iterator it = server.begin(); it != server.end(); it++) 
	{
		if (!(*it)->listenPort) {
			return(1);
		}
		for (std::vector<MultiTree*>::const_iterator treeIt = (*it)->root.begin(); treeIt != (*it)->root.end(); treeIt++)
		{
			if ((*treeIt)->CheckDuplicateError() == false)
				return 1;
		}
		// if (locationUriErrorCheck((*it)->locationList))
		// 		return 1;
		for (std::vector<ServerBlock*>::const_iterator nextIt = it + 1; nextIt != server.end(); nextIt++) {
			if ((*it)->listenPort == (*nextIt)->listenPort)
				return 1;
		}
	}
	return 0;
}

int ParserErrorCheck(HttpBlock &http) {
	//서버블록이 0개일경우
	//서버블록안에 listen이 없을경우, 숫자여야한다, 다른 서버랑 중복이 아니여야한다. 범위는  0~65535
	//로케이션 uri중복인지 아닌지
	if (serverListenErrorCheck(http.serverList)) {
		return 1;
	}

	for (std::vector<MultiTree*>::iterator it = http.root.begin(); it != http.root.end(); it++)
	{
		if ((*it)->CheckDuplicateError() == false)
			return 1;
	}
	return 0;
}

int ParseFile(const std::string &fileName, HttpBlock &http)
{
	std::ifstream file(fileName.c_str());
	std::string line;

	if (!file.is_open())
	{
		std::cout << "Failed to open file" << std::endl;
		return (1);
	}

	while (std::getline(file, line))
	{
		line = RemoveComment(line);
		if (int i = ParseLine(line, file, http)){
			std::cout << "error" << i << '\n';
			return 1;
		}
		// TODO ParseLine 실패 시 모든 동적할당 메모리 해제 후 conf file 에러라고 알리고 프로그램 종료
		// TODO catch 됐을 때 동적할당 해제.
	}
	if (ParserErrorCheck(http)){
		std::cout << "errorCheck" << '\n';
		return 1;
	}

	// printParserResult(http);
	return (0);
}
