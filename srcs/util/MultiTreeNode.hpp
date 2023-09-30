#pragma once
#ifndef MULTITREENODE_H
#define MULTITREENODE_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Colors.hpp"
#include "WebServer.hpp"

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

#endif