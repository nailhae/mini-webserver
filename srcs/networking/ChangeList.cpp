#include "ChangeList.hpp"

#include "Colors.hpp"
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
	// fflags로 CGI 처리와 signal 등 처리 해야함.
	struct kevent target;

	target.ident = ident;
	target.filter = filter;
	target.flags = flags;
	target.fflags = 0;
	if (flags == EV_DELETE)
	{
		for (std::vector<struct kevent>::iterator it = mKeventVector.begin(); it != mKeventVector.begin();)
		{
			if (it->ident == ident && it->filter == filter)
			{
				it = mKeventVector.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	if (filter == EVFILT_TIMER)
	{
		int timerTimeMs = 180000;
		target.data = timerTimeMs;
	}
	else
	{
		target.data = 0;
	}
	target.udata = udata;
	mKeventVector.push_back(target);
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

void ChangeList::PrintEveryList(void)
{
	std::cout << "총 개수: " << mKeventVector.size() << std::endl;
	for (std::vector<struct kevent>::iterator it = mKeventVector.begin(); it != mKeventVector.end(); it++)
	{
		if (it->filter == EVFILT_WRITE)
		{
			std::cout << "[" << it->ident << "] Write ";
			if (it->flags == EV_DELETE)
				std::cout << "delete" << std::endl;
			else if ((it->flags & EV_ADD) == EV_ADD)
				std::cout << "add" << std::endl;
			else if ((it->flags & EV_ENABLE) == EV_ENABLE)
				std::cout << "enable" << std::endl;
			else if ((it->flags & EV_DISABLE) == EV_DISABLE)
				std::cout << "disable" << std::endl;
		}
		if (it->filter == EVFILT_READ)
		{
			std::cout << "[" << it->ident << "] Read ";
			if (it->flags == EV_DELETE)
				std::cout << "delete" << std::endl;
			else if ((it->flags & EV_ADD) == EV_ADD)
				std::cout << "add" << std::endl;
			else if ((it->flags & EV_ENABLE) == EV_ENABLE)
				std::cout << "enable" << std::endl;
			else if ((it->flags & EV_DISABLE) == EV_DISABLE)
				std::cout << "disable" << std::endl;
		}
	}
}