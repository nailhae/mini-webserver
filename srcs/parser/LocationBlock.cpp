#include "MultiTree.hpp"
#include "MultiTreeNode.hpp"
#include "Parser.hpp"

int LocationParser(LocationBlock& location, std::ifstream& file, MultiTree& root, std::string uri)
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
				if (value.at(value.size() - 1) == ';')
				{
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
			if ((iss >> value) && value.at(value.size() - 1) == ';')
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
			if (iss >> value && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
				location.autoindex = (value == "on" ? true : false);
			}
			else
			{
				return 2;
			}
		}
		else if (key == "index")
		{
			std::string value;

			if (iss >> value && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return (2);
			}
			value.erase(value.size() - 1);
			location.index = value;
		}
		else if (key == "alias")
		{
			std::string value;
			if (iss >> value && value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
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
			if (secondPair.at(secondPair.size() - 1) == ';')
			{
				secondPair.erase(secondPair.size() - 1);
			}
			else
			{
				return 2;
			}
			location.returnPair = std::make_pair(firstPair, secondPair);
		}
		else if (key == "location")
		{
			std::string value;
			LocationBlock* locationChild = new LocationBlock;
			InitLocationBlock(*locationChild);
			if (!(iss >> value) || value[0] != '/')
			{
				delete locationChild;
				return 1;
			}
			if (value.at(value.size() - 1) == '/')
			{
				value.erase(value.size() - 1);
			}
			locationChild->uri = value;
			if (!(iss >> value) || value != "{")
			{
				delete locationChild;
				return 1;
			}
			root.searchNodeOrNull(uri)->AddChildNode(locationChild);
			if (LocationParser(*locationChild, file, root, uri + locationChild->uri))
			{
				return (1);
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