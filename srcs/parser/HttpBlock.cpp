#include "parser.hpp"

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
			if (value.back() == ';')
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
			for (int i = 0; i < result.size() - 1; i++)
			{
				// TODO value에 값이 없지 않나?
				http.errorPages[strtol(result[i].c_str(), NULL, 10)] = errorRoute;
				iss >> value;
			}
			iss >> value;
		}
	}
	else if (key == "server")
	{
		// } 가 나올때까지
		std::string value;
		if (!(iss >> value) || value.size() != 1 || value[0] != '{')
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
		if (value.at(value.size() - 1) != '/')
			location->uri = value + '/';
		else
			location->uri = value;
		if (!(iss >> value) || value.size() != 1 || value[0] != '{')
		{
			return 3;
		}
		// } 가 나올때까지
		if (LocationParser(*location, file)){
			return (1);
		} else {
			http.locationList.push_back(location);
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