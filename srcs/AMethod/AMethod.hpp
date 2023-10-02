#pragma once

#include <map>
#include "UserData.hpp"
#include "WebServer.hpp"

enum eSetupFlags
{
	URI = 0x10,
	B_GET_SETTING = 0x20,
	B_POST_SETTING = 0x40,
	B_DELETE_SETTING = 0x80,
	B_HEAD_SETTING = 0x100,
	B_AUTOINDEX = 0x200,
	INDEX_PAGE = 0x400,
	LOCATION_ROOT_PATH = 0x800,
	ALIAS = 0x1000,
	RETURN_PAIR_VEC = 0x2000,
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
