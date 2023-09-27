#ifndef PARSER_H
#define PARSER_H

// #include "../util/Colors.hpp"
// #include "../util/MultiTree.hpp"
// #include "../util/MultiTreeNode.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <vector>
#include <exception>

class MultiTree;
class MultiTreeNode;

enum BlockType
{
	NONE,
	SERVER,
	LOCATION
};

struct LocationBlock;

struct ServerBlock;

struct HttpBlock
{
	std::map<std::string, std::string> types;
	std::map<int, std::string> errorPages;
	std::vector<ServerBlock *> serverList;	   // default_server = vector<ServerBlock> index 0
	// std::vector<LocationBlock *> locationList; // default_server = vector<ServerBlock> index 0
	std::vector<MultiTree *> root;
	int clientMaxBodySize; // default unit kB
	int clientBodyTimeout; // default unit sec.
	int workerConnections;
};

struct ServerBlock
{
	int listenPort;
	std::string serverName;
	std::string rootPath;
	// std::vector<LocationBlock *> locationList;
	std::vector<MultiTree *> root;
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
	// std::vector<LocationBlock *> locationList;
};

void InitHttpBlock(HttpBlock &http);
void InitServerBlock(ServerBlock &server);
void InitLocationBlock(LocationBlock &location);
int ParseFile(const std::string &fileName, HttpBlock &http);
int ServerParser(ServerBlock &server, std::ifstream &file);
int ParseLine(const std::string &line, std::ifstream &file, HttpBlock &http);
std::vector<std::string> split(std::string str);
void InitHttpBlock(HttpBlock &http);
void InitServerBlock(ServerBlock &server);
void InitLocationBlock(LocationBlock &location);

#endif