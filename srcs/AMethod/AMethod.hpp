#pragma once
#include <map>
#include "UserData.hpp"

// TODO 대문자 형식으로 전환
enum eSetupFlags
{
	URI = 0x10,
	B_GET_SETTING = 0x20,
	BPostSETTING = 0x40,
	BDelSETTING = 0x80,
	BHeadSETTING = 0x100,
	BAutoIndex = 0x200,
	IndexPage = 0x400,
	LocationRootPath = 0x800,
	Alias = 0x1000,
	ReturnPairVec = 0x2000,
};

struct ResponseSetup
{
	ServerBlock server;
	LocationBlock location;
};

class AMethod
{
public:
	AMethod(int type);
	virtual ~AMethod();

	virtual int GenerateResponse(UserData&) = 0;
	void ResponseConfigSetup(ServerBlock& server, UserData& target);
	int GetType() const;
	void SetSetupFlag(eSetupFlags flag);
	bool IsFinishSetupFlags(void) const;
	void applySettingLocationBlock(LocationBlock& valueSet, const LocationBlock* valueToSet);

private:
	AMethod();
	AMethod(const AMethod& other);
	AMethod& operator=(const AMethod& other);

	static const int mSetupFinished;
	int mType;
	int mSetupFlags;
};
