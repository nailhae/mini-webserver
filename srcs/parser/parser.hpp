#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <vector>
#include <exception>

struct LocationBlock;

struct ServerBlock;

struct HttpBlock
{
	std::map<std::string, std::string> types;
	std::map<int, std::string> errorPages;
	std::vector<ServerBlock *> serverList;	   // default_server = vector<ServerBlock> index 0
	std::vector<LocationBlock *> locationList; // default_server = vector<ServerBlock> index 0

	int clientMaxBodySize; // default unit kB
	int clientBodyTimeout; // default unit sec.
	int workerConnections;
};

struct ServerBlock
{
	int listenPort; // default port 4242
	std::string serverName;
	std::string rootPath;
	std::vector<LocationBlock *> locationList;
};

struct LocationBlock
{
	std::string uri;
	bool bget;			// default false;
	bool bpost;			// default false;
	bool bdeleteMethod; // default false;
	bool autoindex;		// default false
	std::string index;	// default "index.html"
	std::string rootPath;
	std::string alias;
	std::pair<int, std::string> returnPair;
	std::vector<LocationBlock *> locationList;
};

void InitHttpBlock(HttpBlock &http);
void InitServerBlock(ServerBlock &server);
void InitLocationBlock(LocationBlock &location);
void ParseFile(const std::string &fileName, HttpBlock &http);

#endif