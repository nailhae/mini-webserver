#include "MultiTreeNode.hpp"

MultiTreeNode::MultiTreeNode(locationBlock* data)
	:	mChildren(std::vector<MultiTreeNode *>())
	,	mParentNode(NULL)
	,	mData(data)
{}

MultiTreeNode::~MultiTreeNode(void)
{
	if (mData != NULL)
	{
		delete (mData);
	}
	for (std::vector<MultiTreeNode *>::iterator it = mChildren.begin(); it != mChildren.end(); it++)
	{
		delete (*it);
	}
}

size_t MultiTreeNode::GetChildrenSize(void) const
{
	return (mChildren.size());
}

const locationBlock* MultiTreeNode::GetLocationBlock(void) const
{
	return (mData);
}

const std::string& MultiTreeNode::GetURI(void) const
{
	return (mData->uri);
}

const std::vector<MultiTreeNode *>& MultiTreeNode::GetChildren(void)
{
	return (mChildren);
}

void MultiTreeNode::AddChildNode(locationBlock *target)
{
	MultiTreeNode *child = new MultiTreeNode(target);
	child->mParentNode = this;
	mChildren.push_back(child);
}