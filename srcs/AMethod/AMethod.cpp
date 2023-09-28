#include "AMethod.hpp"
#include <iostream>

AMethod::AMethod()
	: mSetupFinished(GetSetupFinishedValue())
{
}

AMethod::AMethod(int type)
	: mType(type)
	, mSetupFlags(0)
	, mSetupFinished(GetSetupFinishedValue())
{
}

AMethod::~AMethod()
{
	// Destructor implementation
}

AMethod::AMethod(const AMethod& other)
{
	(void)other;
}

AMethod& AMethod::operator=(const AMethod& other)
{
	if (this == &other)
	{
		return *this;
	}

	return *this;
}

void AMethod::ResponseConfigSetup(UserData&)
{
	// 미완 아래 내용은 메모
	// MultiTreeNode* node;
	// searchNodeOrNull

	// 	while (1)
	// {
	// }
	// for (::(users port).getTree 순회)
	// {
	// 	가장 구체적인 루트(매칭되는 길이가 더 긴 uri) search(Userdata.uri)
	// }
	// while (node.parentNode != NULL)
	// {
	// }

	// delete node;
}

int AMethod::GetType() const
{
	return mType;
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

int AMethod::GetSetupFinishedValue(void) const
{
	int setupFinished = 0;

	for (int i = 0; i < SetupFlagCount; i++)
	{
		setupFinished = setupFinished << 1;
		setupFinished |= 1;
	}

	return setupFinished;
}
