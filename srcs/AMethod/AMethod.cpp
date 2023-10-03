#include "AMethod.hpp"
#include "MultiTree.hpp"
#include "UserData.hpp"

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
	if (ALIAS & mSetupFlags)
	{
		valueSet.alias = valueToSet->alias;
		mSetupFlags |= ALIAS;
	}
	if (B_AUTOINDEX & mSetupFlags)
	{
		valueSet.autoindex = valueToSet->autoindex;
		mSetupFlags |= B_AUTOINDEX;
	}
	if ((B_DELETE_SETTING & mSetupFlags) || (B_GET_SETTING & mSetupFlags) || (B_POST_SETTING & mSetupFlags) ||
		(B_HEAD_SETTING & mSetupFlags))
	{
		valueSet.bDeleteMethod = valueToSet->bDeleteMethod;
		mSetupFlags |= B_DELETE_SETTING;
		mSetupFlags |= B_GET_SETTING;
		mSetupFlags |= B_POST_SETTING;
		mSetupFlags |= B_HEAD_SETTING;
	}
	if (INDEX_PAGE & mSetupFlags)
	{
		valueSet.index = valueToSet->index;
		mSetupFlags |= INDEX_PAGE;
	}
	if (LOCATION_ROOT_PATH & mSetupFlags)
	{
		valueSet.rootPath = valueToSet->rootPath;
		mSetupFlags |= LOCATION_ROOT_PATH;
	}
	if (RETURN_PAIR_VEC & mSetupFlags)
	{
		valueSet.returnPair = valueToSet->returnPair;
		mSetupFlags |= RETURN_PAIR_VEC;
	}
}

void AMethod::ResponseConfigSetup(ServerBlock& server, UserData& target)
{
	MultiTree* targetTree = NULL;
	MultiTreeNode* targetTreeNode = NULL;
	const MultiTreeNode* offset = NULL;
	std::string subString;

	for (std::vector<MultiTree*>::const_iterator it = server.root.begin(); it != server.root.end(); it++)
	{
		subString = target.GetUri().substr(0, (*it)->GetRoot()->GetURI().size());
		if ((*it)->GetRoot()->GetURI() == subString)
		{
			if (targetTree == NULL || targetTree->GetRoot()->GetURI().size() < subString.size())
				targetTree = *it;
			break;
		}
	}
	targetTreeNode = targetTree->searchNodeOrNull(target.GetUri());
	// server 블록에서 / 설정이 안 되어 있으면 어떻게 하지? 하긴 해당되는 블록이 없는 경우 허용 메서드 없어서 아무것도
	// 접근 못하긴 함.
	if (targetTreeNode == NULL)
		return;
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
