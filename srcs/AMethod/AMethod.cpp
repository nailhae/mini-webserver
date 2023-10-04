#include "AMethod.hpp"
#include "MultiTree.hpp"
#include "UserData.hpp"

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
	valueSet.uri.insert(0, valueToSet->uri);
	if (!(ALIAS & mSetupFlags))
	{
		valueSet.alias = valueToSet->alias;
		mSetupFlags |= ALIAS;
	}
	if (!(B_AUTOINDEX & mSetupFlags) && valueToSet->autoindex == true)
	{
		valueSet.autoindex = true;
		mSetupFlags |= B_AUTOINDEX;
	}
	if (!((B_DELETE_SETTING & mSetupFlags) || (B_GET_SETTING & mSetupFlags) || (B_POST_SETTING & mSetupFlags) ||
		(B_HEAD_SETTING & mSetupFlags)) \
		&& (valueToSet->bDeleteMethod + valueToSet->bGetMethod + valueToSet->bHeadMethod + valueToSet->bPostMethod > 0))
	{
		valueSet.bDeleteMethod = valueToSet->bDeleteMethod;
		valueSet.bGetMethod = valueToSet->bGetMethod;
		valueSet.bPostMethod = valueToSet->bPostMethod;
		valueSet.bHeadMethod = valueToSet->bHeadMethod;
		mSetupFlags |= B_DELETE_SETTING;
		mSetupFlags |= B_GET_SETTING;
		mSetupFlags |= B_POST_SETTING;
		mSetupFlags |= B_HEAD_SETTING;
	}
	if (!(INDEX_PAGE & mSetupFlags) && valueToSet->index.size() != 0)
	{
		valueSet.index = valueToSet->index;
		mSetupFlags |= INDEX_PAGE;
	}
	if (!(LOCATION_ROOT_PATH & mSetupFlags) && valueToSet->rootPath.size() != 0)
	{
		valueSet.rootPath = valueToSet->rootPath;
		mSetupFlags |= LOCATION_ROOT_PATH;
	}
	if (!(RETURN_PAIR_VEC & mSetupFlags) && valueToSet->returnPair.first != 0)
	{
		valueSet.returnPair = valueToSet->returnPair;
		mSetupFlags |= RETURN_PAIR_VEC;
	}
}

void AMethod::ResponseConfigSetup(const ServerBlock& server, std::string& uri, LocationBlock& setting)
{
	MultiTree* targetTree = NULL;
	MultiTreeNode* targetTreeNode = NULL;
	MultiTreeNode* offset = NULL;
	std::string subString;

	for (std::vector<MultiTree*>::const_iterator it = server.root.begin(); it != server.root.end(); it++)
	{
		subString = uri.substr(0, (*it)->GetRoot()->GetURI().size());
		if ((*it)->GetRoot()->GetURI() == subString)
		{
			if (targetTree == NULL || targetTree->GetRoot()->GetURI().size() < subString.size())
				targetTree = *it;
		}
	}
	targetTreeNode = targetTree->searchNodeOrNull(uri);
	if (targetTreeNode == NULL)
		return;
	offset = targetTreeNode;
	while (offset != NULL)
	{
		applySettingLocationBlock(setting, offset->GetLocationBlock());
		offset = offset->ParentNode();
	}
}

int AMethod::GetType() const
{
	return (mType);
}
