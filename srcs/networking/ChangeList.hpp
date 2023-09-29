#pragma once
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "./UserData.hpp"
#include "../util/Colors.hpp"

#define BUFFER_SIZE 1024

enum eHeaders
{
	NONE,
	CONNECTION,
	CONTENT_TYPE,
	CONTENT_LENGTH,
	CACHE_CONTROL,
	IF_NONE_MATCH,
	IF_MODIFIED_SINCE,
};

enum eMethod
{
	GET = 0,
	HEAD,
	POST,
	DELETE,
	ERROR = -1
};

class ChangeList // 얘는 kqueue manage class가 될 수 있음.
{
public:
	ChangeList(void);
	~ChangeList(void);
	void changeEvent(uintptr_t nIdent, int nFilter, int nFlags);
	void clearEvent(void);
	std::vector<struct kevent>& getKeventVector(void);
	size_t getSize(void);
	size_t getUdata(void);

private:
	ChangeList& operator=(const ChangeList& rhs);
	ChangeList(const ChangeList& other);

	std::vector<struct kevent> _keventVector;
};