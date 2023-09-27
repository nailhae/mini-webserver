#include "parser.hpp"
#include "../util/MultiTree.hpp"
#include "../util/MultiTreeNode.hpp"

int ServerParser(ServerBlock &server, std::ifstream &file)
{
	std::stack<BlockType> blockStack;
	blockStack.push(SERVER);
	std::string line;
	while (getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
        char* pos;
		// std::cout << line << "\n";
		if (!(iss >> key))
		{
			continue;
		}
		if (key[0] == '#')
		{
			continue;
		}
		if (key == "listen")
		{
			std::string value;
			if (iss >> value)
			{
				if (value.back() == ';')
				{
					value.erase(value.size() - 1);
				}
                if (value.empty())
                    return 1;
				server.listenPort = strtol(value.c_str(), &pos, 10);
                if (*pos)
                    return (1);
                if (server.listenPort < 0 && 65535 > server.listenPort)
                    return (1);
			}
			if ((iss >> value) && value != "default_server;") {
				return 1;
			}
		}
		else if (key == "server_name")
		{
			std::string value;
			if ((iss >> value) && value.back() == ';')
			{
				value.erase(value.size() - 1);
			}
			else
			{
				return 3;
			}
			server.serverName = value;
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
				return 3;
			}
			if (!value.empty())
			{
				server.rootPath = value + '/';
			}
			else
			{
				return 3;
			}
		}
		else if (key == "location")
		{
			std::string value;
			if (!(iss >> value) || value[0] != '/')
			{
				return 3;
			}
			LocationBlock *location = new LocationBlock;
			InitLocationBlock(*location);
			MultiTreeNode *temp = new MultiTreeNode(location);
			MultiTree *tree = new MultiTree(*temp);
			server.root.push_back(tree);
			if (value.at(value.size() - 1) == '/')
			{
				value.erase(value.size() - 1);
			}
			location->uri = value;
			if (!(iss >> value) || value != "{")
			{
				return 3;
			}
			if (LocationParser(*location, file, *server.root.at(server.root.size() - 1), location->uri) != 0)
			{
				return (3);
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
	}
	return (1);
}
