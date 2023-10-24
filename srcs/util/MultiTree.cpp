#include "MultiTree.hpp"

#include "Error.hpp"

MultiTree::MultiTree(MultiTreeNode& root)
	: mRoot(&root)
{
}

MultiTree::MultiTree(const MultiTree& other)
{
	this->mRoot = other.mRoot;
}

MultiTree& MultiTree::operator=(const MultiTree& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}
	this->mRoot = rhs.mRoot;
	return *this;
}

MultiTree::~MultiTree(void)
{
	delete (mRoot);
}

/**
 * @brief 재귀적으로 노드의 모든 자식의 uri를 비교하여 가장 깊은 노드를 찾아내는 함수
 *
 * @param node 해당 노드
 * @param target 하위 경로의 uri
 * @return MultiTreeNode*
 *
 */
static MultiTreeNode* findNodeOrNullRecursive(MultiTreeNode* node, std::string& target)
{
	MultiTreeNode* uritemp = NULL;
	MultiTreeNode* result = NULL;
	std::string subString;

	if (node == NULL)
		return (NULL);
	else if (node->GetURI() != target.substr(0, node->GetURI().size()))
	{
		return (NULL);
	}
	target.erase(0, node->GetURI().size());
	if (target.size() == 0)
		return (node);
	for (std::vector<MultiTreeNode*>::const_iterator it = node->GetChildren().begin(); it != node->GetChildren().end();
		 it++)
	{
		subString = target.substr(0, (*it)->GetURI().size());
		if ((*it)->GetURI() == subString)
		{
			if (uritemp == NULL || uritemp->GetURI().size() < subString.size())
				uritemp = *it;
		}
	}
	result = findNodeOrNullRecursive(uritemp, target);
	if (result == NULL)
	{
		return (node);
	}
	else
	{
		return (result);
	}
}

MultiTreeNode* MultiTree::GetRoot(void) const
{
	return (mRoot);
}

/**
 * @brief uri를 string으로 받아 해당하는 최대 깊이의 트리 노드를 반환하는 함수
 *
 * @param target uri의 string
 * @return MultiTreeNode* 실패시: NULL
 */
MultiTreeNode* MultiTree::searchNodeOrNull(std::string target) const
{
	return (findNodeOrNullRecursive(mRoot, target));
}

void printSearchedResult(MultiTree& root, std::string uri)
{
	MultiTreeNode* uritemp;

	uritemp = root.searchNodeOrNull(uri);
	if (uritemp == NULL)
		Error::Print("failed to find " + uri);

	else
		Error::Print("found node's URI: " + uritemp->GetURI());
}

static void printNodeRecursive(MultiTreeNode* node)
{
	if (node == NULL)
		return;
	node->PrintData();
	for (std::vector<MultiTreeNode*>::const_iterator it = node->GetChildren().begin(); it != node->GetChildren().end();
		 it++)
	{
		printNodeRecursive(*it);
	}
}

void MultiTree::PrintEveryNodes(void)
{
	printNodeRecursive(mRoot);
}

static int CheckDupRecursive(MultiTreeNode* node)
{
	std::string uritemp;

	if (node == NULL || node->GetChildrenSize() < 2)
		return (true);
	for (std::vector<MultiTreeNode*>::const_iterator it = node->GetChildren().begin(); it != node->GetChildren().end();
		 it++)
	{
		uritemp = (*it)->GetURI();
		for (std::vector<MultiTreeNode*>::const_iterator nextIt = it + 1; nextIt != node->GetChildren().end(); nextIt++)
		{
			if (uritemp == (*nextIt)->GetURI())
				return (false);
		}
		if (CheckDupRecursive(*it) == false)
			return (false);
	}
	return (true);
}

int MultiTree::CheckDuplicateError(void)
{
	return (CheckDupRecursive(mRoot));
}