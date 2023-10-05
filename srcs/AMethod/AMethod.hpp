#pragma once

#include <map>
#include "dataSet.hpp"

class AMethod
{
public:
	AMethod(int type);
	virtual ~AMethod();

	virtual int GenerateResponse(UserData&) = 0;
	void ResponseConfigSetup(const ServerBlock& server, std::string& uri, LocationBlock& setting);
	int GetType() const;
	void SetSetupFlag(eSetupFlags flag);
	bool IsFinishSetupFlags(void) const;
	void applySettingLocationBlock(LocationBlock& valueSet, const LocationBlock* valueToSet);

private:
	AMethod();
	AMethod(const AMethod& other);
	AMethod& operator=(const AMethod& other);

	int mType;
	int mSetupFlags;
};
