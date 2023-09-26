#include "parser.hpp"

int LocationParser_2(LocationBlock &location, std::ifstream &file)
{
	std::stack<BlockType> blockStack;
	blockStack.push(LOCATION);
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
		if (!(iss >> key))
		{
			continue;
		}
		if (key[0] == '#')
		{
			continue;
		}
		if (key == "limit_except")
		{
			std::string value;
			while ((iss >> value))
			{
				if (value.back() == ';') {
					value.erase(value.size() - 1);
				}
				if (value == "GET")
				{
					location.bget = true;
				}
				else if (value == "POST")
				{
					location.bpost = true;
				}
				else if (value == "DELETE")
				{
					location.bdeleteMethod = true;
				}
			}
		}
		else if (key == "root")
		{
			std::string value;
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 2;
			}
			if (!(value.empty()))
			{
				location.rootPath = value + '/';
			}
			else
			{
				return 2;
			}
		}
		else if (key == "autoindex")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
				location.autoindex = (value == "on" ? true : false);
			} else {
				return 2;
			}
		}
		else if (key == "index")
		{
			std::string value;

			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			value.erase(value.size() - 1);
			location.index = value;
		}
		else if (key == "alias")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			location.alias = value;
		}
		else if (key == "return")
		{
			int firstPair = 0;
			std::string secondPair = "";
			iss >> firstPair;
			iss >> secondPair;
			if (secondPair.back() == ';')
			{
				secondPair.erase(secondPair.size() - 1);
			}
			else
			{
				return 2;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		} else if (key == "location") {
			std::string value;
			LocationBlock *location_mini = new LocationBlock;
			InitLocationBlock(*location_mini);
			if (!(iss >> value) || value[0] != '/')
			{
				return 1;
			}
			location_mini->uri = value;
			if (!(iss >> value) || value.size() != 1 || value[0] != '{')
			{
				return 1;
			}
			// if (LocationParser(*location_mini, file)){
			// 	std::cout << "loc3 " << '\n';
			// 	return (1);
			// } else {
			// 	location.locationList.push_back(location_mini);
			// }
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return (0);
			}
		}
		else
		{
			return 2;
		}
	}
	return 0;
}

int LocationParser(LocationBlock &location, std::ifstream &file)
{
	std::stack<BlockType> blockStack;
	blockStack.push(LOCATION);
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
		if (!(iss >> key))
		{
			continue;
		}
		if (key[0] == '#')
		{
			continue;
		}
		if (key == "limit_except")
		{
			std::string value;
			while ((iss >> value))
			{
				if (value.back() == ';') {
					value.erase(value.size() - 1);
				}
				if (value == "GET")
				{
					location.bget = true;
				}
				else if (value == "POST")
				{
					location.bpost = true;
				}
				else if (value == "DELETE")
				{
					location.bdeleteMethod = true;
				}
			}
		}
		else if (key == "root")
		{
			std::string value;
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 2;
			}
			if (!(value.empty()))
			{
				location.rootPath = value + '/';
			}
			else
			{
				return 2;
			}
		}
		else if (key == "autoindex")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
				location.autoindex = (value == "on" ? true : false);
			} else {
				return 2;
			}
		}
		else if (key == "index")
		{
			std::string value;

			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			value.erase(value.size() - 1);
			location.index = value;
		}
		else if (key == "alias")
		{
			std::string value;
			if (iss >> value && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else {
				return (2);
			}
			location.alias = value;
		}
		else if (key == "return")
		{
			int firstPair = 0;
			std::string secondPair = "";
			iss >> firstPair;
			iss >> secondPair;
			if (secondPair.back() == ';')
			{
				secondPair.erase(secondPair.size() - 1);
			}
			else
			{
				return 2;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		} else if (key == "location") {
			std::string value;
			LocationBlock *location_mini = new LocationBlock;
			InitLocationBlock(*location_mini);
			if (!(iss >> value) || value[0] != '/')
			{
				return 1;
			}
			if (value.back() != '/')
				location_mini->uri = value + '/';
			else
				location_mini->uri = value;
			if (!(iss >> value) || value.size() != 1 || value[0] != '{')
			{
				return 1;
			}
			if (LocationParser_2(*location_mini, file)){
				return (1);
			} else {
				// location_mini->locationList.push_back(location_mini);
				location.locationList.push_back(location_mini);
			}
		}
		else if (key == "}")
		{
			blockStack.pop();
			if (blockStack.empty())
			{
				return (0);
			}
		}
		else
		{
			return 2;
		}
	}
	return 0;
}
