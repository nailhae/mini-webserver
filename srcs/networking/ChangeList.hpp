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

#include "dataSet.hpp"

class ChangeList
{
public:
	ChangeList(void);
	~ChangeList(void);
	void ChangeEvent(uintptr_t nIdent, int nFilter, int nFlags, UserData* udata);
	void ClearEvent(void);
	std::vector<struct kevent>& GetKeventVector(void);
	void setKq(int kq);
	int getKq(void);
	size_t GetSize(void);
	size_t GetUdata(void);

private:
	ChangeList& operator=(const ChangeList& rhs);
	ChangeList(const ChangeList& other);

	int mKq;
	std::vector<struct kevent> mKeventVector;
};