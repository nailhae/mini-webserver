#include "Error.hpp"

void Error::Print(const std::string& message)
{
	std::cerr << Colors::RedString("Error: " + message) << std::endl;
}

void Error::CheckLeak(int num)
{
	std::cout << "------------" << num << "------------" << std::endl;
	system("leaks webserv");
}