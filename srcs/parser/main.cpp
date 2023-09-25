#include "parser.hpp"

int main()
{
	HttpBlock *config = new HttpBlock;
	InitHttpBlock(*config);
	// ParseFile("nginx.conf", *config);
	ParseFile("default.conf", *config);
}