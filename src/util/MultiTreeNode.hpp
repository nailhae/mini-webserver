#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief 임시로 사용할 location block
 * 
 * merge 후 삭제할 struct
 */
typedef struct locationBlock
{
	std::string 				uri;
	bool 						get; /*= false;*/
	bool 						post;/*= false;*/
	bool 						del; /*= false;*/
	bool 						autoIndex; /*= false;*/
	std::string 				index; /*= "index.html";*/
	std::string 				alias; // alias 경로
	std::pair<int, std::string>	return_pair; // <statusCode, uri>. redirection도 포함됨
} locationBlock;

class MultiTreeNode
{
public:

	MultiTreeNode(locationBlock* data);
	~MultiTreeNode(void);
	size_t GetChildrenSize(void) const;
	const locationBlock* GetLocationBlock(void) const;
	const std::string& GetURI(void) const;
	const std::vector<MultiTreeNode*>& GetChildren(void);
	void AddChildNode(locationBlock *target);

private:

	MultiTreeNode(void);
	MultiTreeNode& operator=(const MultiTreeNode& rhs);
	MultiTreeNode(const MultiTreeNode& other);

	std::vector<MultiTreeNode*> mChildren;
	MultiTreeNode*				mParentNode;
	locationBlock*				mData;

};