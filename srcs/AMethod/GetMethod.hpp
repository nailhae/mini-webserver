#pragma once
#include "AMethod.hpp"

class GetMethod : public AMethod
{
public:
	GetMethod(int type);
	~GetMethod(void);
	int GenerateResponse(UserData&);
private:
	const GetMethod& operator=(const GetMethod& rhs);
	GetMethod(const GetMethod& rhs);
	GetMethod(void);
};