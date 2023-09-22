#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <stack>
#include <stdlib.h> 

struct LocationBlock;

struct ServerBlock;

struct HttpBlock {
	std::map<std::string, std::string> types;
	std::map<int, std::string> errorPages;
	std::vector<ServerBlock> serverList;
	
	int clientMaxBodySize; // default unit kB
	int clientBodyTimeout = 60; // default unit sec.
	int workerConnections = 1024;
};

struct ServerBlock {
	int listenPort; // default port 4242
	std::string serverName;
	std::string rootPath;
	std::vector<LocationBlock> locationList;
};

struct LocationBlock {
	std::string uri;
	bool bget; // default false;
	bool bpost;	// default false;
	bool bdeleteMethod; // default false;
	std::string index; // default "index.html"
	std::string alias;
	std::pair<int, std::string> returnPair;
	std::vector<LocationBlock> locationList;
};

void InitHttpBlock(HttpBlock& http);
void InitServerBlock(ServerBlock& server);
void InitLocationBlock(LocationBlock& location);

#endif