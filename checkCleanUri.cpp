#include <string>
#include <iostream>

bool	checkCleanUri(std::string& input)
{
	if (input.find("/../") != std::string::npos || input.find("//") != std::string::npos)
	{
		return false;
	}
	for (std::string::iterator it = input.begin() + 1; it!= input.end(); it++)
	{
		char c = *it;
		if (!(isalnum(c) || c == '/' || c == '-' || c == '_' || c == '.' || c == '~'))
		{
			/*인코딩 된 문자 있나? ->일단 처리 안하기로.
			 * query'?' fragment'#' 처리 안함.
			 */
			return false;
		}
	}
	size_t	pos = input.find("/./");
	while (pos != std::string::npos)
	{
		input.erase(pos, 2);
		pos = input.find("/./");
	}
	return true;
}

//int	main(void)
//{
//	std::string path1 = "/user_profile/-____--.....42";
//	std::string path2 = "/images/logo.jpg";
//	std::string path3 = "/home~user";
//	std::string path4 = "user/home/hi"; //유효한 케이스로
//	std::string path5 = "/user?name=John";
//	std::string path6 = "/../senstive_file";
//	std::string path7 = "/./././user_profile";
//
//
//	std::cout << checkCleanUri(path1) << path1 << std::endl;  // 1 (true)
//	std::cout << checkCleanUri(path2) << path2 <<std::endl;//1
//	std::cout << checkCleanUri(path3) << path3 <<std::endl; //1
//	std::cout << checkCleanUri(path4) << path4 <<std::endl; //1
//	std::cout << checkCleanUri(path5) << path5 <<std::endl; //0
//	std::cout << checkCleanUri(path6) << path6 <<std::endl; //0
//	std::cout << checkCleanUri(path7) << path7 <<std::endl; //1
//
//	return 0;
//}