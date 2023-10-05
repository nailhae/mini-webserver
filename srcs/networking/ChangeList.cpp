#include "ChangeList.hpp"

#include "Colors.hpp"
#include "UserData.hpp"
#include "dataSet.hpp"

ChangeList::ChangeList(void)
	: _keventVector(std::vector<struct kevent>())
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
	target.data = 0;
	if (flags == EV_DELETE)
	{
		kevent(ident, &target, 1, NULL, 0, 0);
	}
	else
	{
		if ((filter & EVFILT_WRITE) == true)
			std::cout << Colors::BoldRed << "write 킨다" << Colors::Reset << std::endl;
		target.udata = udata;
		_keventVector.push_back(target);
	}
}

void ChangeList::ClearEvent(void)
{
	_keventVector.clear();
}

std::vector<struct kevent>& ChangeList::GetKeventVector(void)
{
	return (_keventVector);
}

size_t ChangeList::GetSize(void)
{
	return (_keventVector.size());
}
