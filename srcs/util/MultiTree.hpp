#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "MultiTreeNode.hpp"

class MultiTree
{
public:
	MultiTree(MultiTreeNode& root);
	~MultiTree(void);
	MultiTreeNode* GetRoot(void) const;
	MultiTreeNode* searchNodeOrNull(std::string target) const;
private:
	MultiTree(void);
	MultiTree& operator=(const MultiTree& rhs);
	MultiTree(const MultiTree& other);

	MultiTreeNode* mRoot;
};