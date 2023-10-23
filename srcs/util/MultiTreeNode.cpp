#include "MultiTreeNode.hpp"

#include "Error.hpp"

MultiTreeNode::MultiTreeNode(LocationBlock* data)
	: mChildren(std::vector<MultiTreeNode*>())
	, mParentNode(NULL)
	, mData(data)
{
}

MultiTreeNode::~MultiTreeNode(void)
{
	if (mData != NULL)
	{
		delete (mData);
	}
	for (std::vector<MultiTreeNode*>::iterator it = mChildren.begin(); it != mChildren.end(); it++)
	{
		delete (*it);
	}
}

size_t MultiTreeNode::GetChildrenSize(void) const
{
	return (mChildren.size());
}

const LocationBlock* MultiTreeNode::GetLocationBlock(void) const
{
	return (mData);
}

const std::string& MultiTreeNode::GetURI(void) const
{
	return (mData->uri);
}

const std::vector<MultiTreeNode*>& MultiTreeNode::GetChildren(void)
{
	return (mChildren);
}

MultiTreeNode* MultiTreeNode::ParentNode(void)
{
	return (mParentNode);
}

std::vector<MultiTreeNode*>& MultiTreeNode::Children(void)
{
	return (mChildren);
}

void MultiTreeNode::AddChildNode(LocationBlock* target)
{
	MultiTreeNode* child = new MultiTreeNode(target);
	child->mParentNode = this;
	mChildren.push_back(child);
}

void MultiTreeNode::PrintData(void) const
{
	std::cout << "URI: " << mData->uri << "\n"
			  << "GET: " << mData->bGetMethod << "\n"
			  << "POST: " << mData->bPostMethod << "\n"
			  << "DELETE: " << mData->bDeleteMethod << "\n"
			  << "HEAD: " << mData->bHeadMethod << "\n"
			  << "autoindex: " << mData->autoindex << "\n"
			  << "index: " << mData->index << "\n"
			  << "alias: " << mData->alias << "\n"
			  << "root: " << mData->rootPath << "\n"
			  << "return: " << mData->returnPair.first << ": " << mData->returnPair.second
			  << "\n--------------------------------" << std::endl;
}

void addChildURI(MultiTreeNode* nodeOrNull, std::string uri)
{
	if (nodeOrNull == NULL)
	{
		Error::Print("node is NULL!");
		return;
	}
	LocationBlock* data = new LocationBlock;
	data->uri = uri;
	nodeOrNull->AddChildNode(data);
	std::cout << Colors::Green << "complete add " << uri << Colors::Reset << std::endl;
}