#include "ChangeList.hpp"

ChangeList::ChangeList(void)
	:	_keventVector(std::vector<struct kevent>())
{}

ChangeList::~ChangeList(void) 
{}

void ChangeList::changeEvent(uintptr_t ident, int filter, int flags)
{
	// fflags로 CGI 처리와 signal 등 처리 해야함.
	struct kevent	target;
	UserData *udata;
	/* init udata */

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
		udata = new UserData(ident);
		target.udata = udata;
		_keventVector.push_back(target);
	}
}

void ChangeList::clearEvent(void)
{
	_keventVector.clear();
}

std::vector<struct kevent>& ChangeList::getKeventVector(void)
{
	return (_keventVector);
}

size_t ChangeList::getSize(void)
{
	return (_keventVector.size());
}
