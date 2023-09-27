#include "parser.hpp"
#include "../util/MultiTree.hpp"
#include "../util/MultiTreeNode.hpp"

int ParseLine(const std::string &line, std::ifstream &file, HttpBlock &http)
{
	std::istringstream iss(line);
	std::string key;

	if (!(iss >> key) || key[0] == '#')
	{
		return (0);
	}
	if (key == "default_type")
	{
		std::string value;
		if (iss >> value)
		{
			if (value.at(value.size() - 1) == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 1;
			}
			http.types["default_type"] = value;
		}
	}
	else if (key == "client_max_body_size")
	{
		std::string value;
		if (iss >> value)
		{
			if (value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 1;
			}
			if (value.back() == 'm' || value.back() == 'M')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024;
			}
			else if (value.back() == 'g' || value.back() == 'G')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10) * 1024 * 1024;
			}
			else if (value.back() == 'k' || value.back() == 'K')
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
			}
			else
			{
				http.clientMaxBodySize = strtol(value.c_str(), NULL, 10);
			}
		}
	}
	else if (key == "error_page")
	{
		std::vector<std::string> result = split(line);
		std::string value;
		if (!(result.empty()))
		{
			std::string errorRoute = result.back();
			if (errorRoute.back() == ';')
			{
				errorRoute.erase(errorRoute.size() - 1);
			}
			else
			{
				return (1);
			}
			for (size_t i = 0; i < result.size() - 1; i++)
			{
				http.errorPages[strtol(result[i].c_str(), NULL, 10)] = errorRoute;
				iss >> value;
			}
			iss >> value;
		}
	}
	else if (key == "server")
	{
		std::string value;
		if (!(iss >> value) || value != "{")
		{
			return 1;
		}
		ServerBlock *server = new ServerBlock;
		InitServerBlock(*server);
		if (ServerParser(*server, file)) {
			return (2);
		}
		http.serverList.push_back(server);
	}
	else if (key == "location")
	{
		std::string value;
		// TODO uri checker가 만들어지면 uri 확인하는 코드도 추가 필요
		if (!(iss >> value) || value[0] != '/')
		{
			return 1;
		}
		LocationBlock *location = new LocationBlock;
		InitLocationBlock(*location);
		MultiTreeNode *temp = new MultiTreeNode(location);
		MultiTree *tree = new MultiTree(*temp);
		http.root.push_back(tree);
		if (value.at(value.size() - 1) == '/')
		{
			value.erase(value.size() - 1);
		}
		location->uri = value;
		if (!(iss >> value) || value != "{")
		{
			return 3;
		}
		if (LocationParser(*location, file, *http.root.at(http.root.size() - 1), location->uri)){
			return (1);
		}
	} else {
		return (1);
	}

	if (iss >> key && key[0] != '#')
	{
		return 1;
	}

	return 0;
}