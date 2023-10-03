#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Colors.hpp"
#include "MultiTreeNode.hpp"
#include "WebServer.hpp"

struct LocationBlock;

class MultiTreeNode;

class MultiTree
{
public:
	MultiTree(MultiTreeNode& root);
	~MultiTree(void);
	MultiTreeNode* GetRoot(void) const;
	MultiTreeNode* searchNodeOrNull(std::string target) const;
	MultiTree& operator=(const MultiTree& rhs);
	MultiTree(const MultiTree& other);
	void PrintEveryNodes(void);
	int CheckDuplicateError(void);

private:
	MultiTree(void);
	MultiTreeNode* mRoot;
};

void printSearchedResult(MultiTree& root, std::string uri);
int LocationParser(LocationBlock& location, std::ifstream& file, MultiTree& root, std::string uri);
