#include "parser.hpp"

int main()
{
	HttpBlock *config = new HttpBlock;
	InitHttpBlock(*config);
	// ParseFile("nginx.conf", *config);
	if (ParseFile("default.conf", *config))
		return 0;
}