#pragma once

#include "AMethod.hpp"

class MethodGet : public AMethod
{
public:
	// temp
	MethodGet(int type)
		: AMethod(type)
	{
	}

	// temp
	~MethodGet(void)
	{
	}

	// temp
	int GenerateResponse(UserData& userData)
	{
		const int temp = 0;
		(void)userData;
		return temp;
	}

private:
	const MethodGet& operator=(const MethodGet& rhs);
	MethodGet(const MethodGet& rhs);
	MethodGet(void);
};