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
	HOST,
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
	void ChangeEvent(uintptr_t nIdent, int nFilter, int nFlags, int socketType);
	void ClearEvent(void);
	std::vector<struct kevent>& GetKeventVector(void);
	size_t GetSize(void);
	size_t GetUdata(void);

private:
	ChangeList& operator=(const ChangeList& rhs);
	ChangeList(const ChangeList& other);

	std::vector<struct kevent> _keventVector;
};