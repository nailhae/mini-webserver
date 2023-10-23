#include "ChangeList.hpp"

#include "Colors.hpp"
#include "Error.hpp"
#include "UserData.hpp"
#include "dataSet.hpp"

ChangeList::ChangeList(void)
	: mKeventVector(std::vector<struct kevent>())
{
}

ChangeList::~ChangeList(void)
{
}

void ChangeList::ChangeEvent(uintptr_t ident, int filter, int flags, UserData* udata)
{
	struct kevent target;

	target.ident = ident;
	target.filter = filter;
	target.flags = flags;
	target.fflags = 0;
	target.udata = NULL;
	if (flags == EV_DELETE)
	{
		kevent(mKq, &target, 1, NULL, 0, 0);
		return;
	}
	else if (filter == EVFILT_TIMER)
	{
		// int timerTimeMs = 200000;
		int timerTimeMs = 10000;
		target.data = timerTimeMs;
	}
	else
	{
		target.data = 0;
	}
	target.udata = udata;
	mKeventVector.push_back(target);
}

void ChangeList::setKq(int kq)
{
	mKq = kq;
}

int ChangeList::getKq(void)
{
	return (mKq);
}

void ChangeList::ClearEvent(void)
{
	mKeventVector.clear();
}

std::vector<struct kevent>& ChangeList::GetKeventVector(void)
{
	return (mKeventVector);
}

size_t ChangeList::GetSize(void)
{
	return (mKeventVector.size());
}
