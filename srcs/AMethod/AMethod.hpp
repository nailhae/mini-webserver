#pragma once
#include <map>
#include "../networking/UserData.hpp"

enum eSetupFlags
{
	ListenPort = 0x01,
	ServerName = 0x02,
	ServerRootPath = 0x04,
	ServerRootVec = 0x08,
	Uri = 0x10,
	BGetMethod = 0x20,
	BPostMethod = 0x40,
	BDelMethod = 0x80,
	BHeadMethod = 0x100,
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
