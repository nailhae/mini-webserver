#include "./AMethod.hpp"
#include "../util/MultiTree.hpp"

const int AMethod::mSetupFinished = 0x3FFF;

AMethod::AMethod()
{
}

AMethod::AMethod(int type)
	: mType(type)
	, mSetupFlags(0)
{
}

AMethod::~AMethod()
{
}

void AMethod::applySettingLocationBlock(LocationBlock& valueSet, const LocationBlock* valueToSet)
{
	if (Alias & mSetupFlags)
	{
		valueSet.alias = valueToSet->alias;
		mSetupFlags |= Alias;
	}
	if (BAutoIndex & mSetupFlags)
	{
		valueSet.autoindex = valueToSet->autoindex;
		mSetupFlags |= BAutoIndex;
	}
	if ((BDelMethod & mSetupFlags) || (BGetMethod & mSetupFlags) || \
		(BPostMethod & mSetupFlags) || (BHeadMethod & mSetupFlags))
	{
		valueSet.bdelete = valueToSet->bdelete;
		mSetupFlags |= BDelMethod;
		mSetupFlags |= BGetMethod;
		mSetupFlags |= BPostMethod;
		mSetupFlags |= BHeadMethod;
	}
	if (IndexPage & mSetupFlags)
	{
		valueSet.index = valueToSet->index;
		mSetupFlags |= IndexPage;
	}
	if (LocationRootPath & mSetupFlags)
	{
		valueSet.rootPath = valueToSet->rootPath;
		mSetupFlags |= LocationRootPath;
	}
	if (ReturnPairVec & mSetupFlags)
	{
		valueSet.returnPair = valueToSet->returnPair;
		mSetupFlags |= ReturnPairVec;
	}
}

void AMethod::ResponseConfigSetup(ServerBlock& server, UserData& target)
{
	MultiTree* targetTree = NULL;
	MultiTreeNode* targetTreeNode = NULL;
	const MultiTreeNode* offset = NULL;
	std::string	subString;
	
	for (std::vector<MultiTree*>::const_iterator it = server.root.begin(); it != server.root.end(); it++)
	{
		subString = target.GetUri().substr(0, (*it)->GetRoot()->GetURI().size());
		if ((*it)->GetRoot()->GetURI() == subString)
		{
			if (targetTree == NULL || targetTree->GetRoot()->GetURI().size() < subString.size())
				targetTree = *it;
			break ;
		}
	}
	targetTreeNode = targetTree->searchNodeOrNull(target.GetUri());
	//server 블록에서 / 설정이 안 되어 있으면 어떻게 하지? 하긴 해당되는 블록이 없는 경우 허용 메서드 없어서 아무것도 접근 못하긴 함.
	if (targetTreeNode == NULL)
		return ;
	offset = targetTreeNode;
	while (offset != NULL)
	{
		applySettingLocationBlock(target.Setting(), offset->GetLocationBlock());
		offset = targetTreeNode->GetParentNode();
	}
}

int AMethod::GetType() const
{
	return (mType);
}

void AMethod::SetSetupFlag(eSetupFlags flag)
{
	mSetupFlags |= flag;
}

bool AMethod::IsFinishSetupFlags(void) const
{
	if (mSetupFlags != mSetupFinished)
	{
		return false;
	}
	return true;
}
