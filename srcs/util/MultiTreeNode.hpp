#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Colors.hpp"
#include "dataSet.hpp"

struct LocationBlock;

class MultiTreeNode
{
public:
	MultiTreeNode(LocationBlock* data);
	~MultiTreeNode(void);
	size_t GetChildrenSize(void) const;
	const LocationBlock* GetLocationBlock(void) const;
	const std::string& GetURI(void) const;
	const std::vector<MultiTreeNode*>& GetChildren(void);
	MultiTreeNode* ParentNode(void);
	std::vector<MultiTreeNode*>& Children(void);
	void AddChildNode(LocationBlock* target);
	void PrintData(void) const;

private:
	MultiTreeNode(void);
	MultiTreeNode& operator=(const MultiTreeNode& rhs);
	MultiTreeNode(const MultiTreeNode& other);

	std::vector<MultiTreeNode*> mChildren;
	MultiTreeNode* mParentNode;
	LocationBlock* mData;
};

void addChildURI(MultiTreeNode* nodeOrNull, std::string uri);
