#pragma once

#include <iostream>
#include "Colors.hpp"

class Error
{
public:
	static void Print(const std::string& message);

private:
	Error();
	virtual ~Error();
	Error(const Error& other);
	Error& operator=(const Error& other);
};
