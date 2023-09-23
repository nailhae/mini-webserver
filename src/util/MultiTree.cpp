#include "MultiTree.hpp"

MultiTree::MultiTree(MultiTreeNode& root)
	: mRoot(&root)
{}

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
	MultiTreeNode*	temp = NULL;
	MultiTreeNode*	result = NULL;
	std::string		subString;

	if (node == NULL)
		return (NULL);
	else if (node->GetURI() != target.substr(0, node->GetURI().size()))
		return (NULL);
	target.erase(0, node->GetURI().size());
	if (target.size() == 0)
		return (node);
	for (std::vector<MultiTreeNode*>::const_iterator it = node->GetChildren().begin(); it != node->GetChildren().end(); it++)
	{
		subString = target.substr(0, (*it)->GetURI().size());
		if ((*it)->GetURI() == subString)
		{
			if (temp == NULL || temp->GetURI().size() < subString.size())
				temp = *it;
		}
	}
	result = findNodeOrNullRecursive(temp, target);
	if (result == NULL)
		return (node);
	else
		return (result);
}

MultiTreeNode* MultiTree::GetRoot(void)
{
	return (mRoot);
}

/**
 * @brief uri를 string으로 받아 해당하는 최대 깊이의 트리 노드를 반환하는 함수
 * 
 * @param target uri의 string
 * @return MultiTreeNode* 실패시: NULL 
 */
MultiTreeNode* MultiTree::searchNodeOrNull(std::string target)
{
	return (findNodeOrNullRecursive(mRoot, target));
}