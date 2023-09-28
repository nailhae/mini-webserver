#ifndef AMETHOD_HPP
#define AMETHOD_HPP

#include "UserData.hpp"
#include "parser.hpp"

enum eSetupFlags
{
	ListenPort = 1 << 0,
	ServerName = 1 << 1,
	ServerRootPath = 1 << 2,
	ServerRootVec = 1 << 3,
	Uri = 1 << 4,
	BGetMethod = 1 << 5,
	BPostMethod = 1 << 6,
	BDelMethod = 1 << 7,
	BAutoIndex = 1 << 7,
	IndexPage = 1 << 7,
	LocationRootPath = 1 << 7,
	Alias = 1 << 7,
	ReturnPairVec = 1 << 7,
	SetupFlagCount = 13
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
	void ResponseConfigSetup(UserData&);
	int GetType() const;
	void SetSetupFlag(eSetupFlags flag);
	bool IsFinishSetupFlags(void) const;

private:
	AMethod();
	AMethod(const AMethod& other);
	AMethod& operator=(const AMethod& other);

	int GetSetupFinishedValue(void) const;

	int mType;
	int mSetupFlags;
	const int mSetupFinished;
};

#endif /* AMETHOD_HPP */
