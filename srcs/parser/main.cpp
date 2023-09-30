#include "MultiTree.hpp"
#include "MultiTreeNode.hpp"
#include "Parser.hpp"

void PrintTreeStructure(MultiTreeNode* node, int depth = 0)
{
	if (node == nullptr)
	{
		return;
	}
	// 현재 노드 정보 출력
	const LocationBlock* data = node->GetLocationBlock();
	std::cout << std::string(depth, ' ') << "URI: " << data->uri << std::endl;
	// 자식 노드 순회 및 출력
	for (std::vector<MultiTreeNode*>::const_iterator it = node->GetChildren().begin(); it != node->GetChildren().end();
		 it++)
	{
		PrintTreeStructure((*it), depth + 2); // 들여쓰기 레벨 조절
	}
}

// void RecurFreeLocationBlock(LocationBlock** targetLocation)
// {
// 	if ((*targetLocation)->locationList.size() == 0)
// 	{
// 		delete *targetLocation;
// 		return ;
// 	}
// 	for (std::vector<LocationBlock *>::iterator it = (*targetLocation)->locationList.begin(); \
// 		it != (*targetLocation)->locationList.end(); it++)
// 	{
// 		RecurFreeLocationBlock(&(*it));
// 	}
// 	delete *targetLocation;
// }

// void FreeServerBlock(ServerBlock** targetServer)
// {
// 	for (std::vector<MultiTree*>::iterator it = (*targetServer)->root.begin(); \
// 		it != (*targetServer)->root.end(); it++)
// 	{
// 		delete *it;
// 		// RecurFreeLocationBlock(&(*it));
// 	}
// 	delete *targetServer;
// }

void leak()
{
	system("leaks $PPID");
}

void deleteHttpBlock(HttpBlock& target)
{
	for (std::vector<MultiTree*>::iterator it = target.root.begin(); it != target.root.end(); it++)
	{
		delete (*it);
	}
	for (std::vector<ServerBlock*>::iterator it = target.serverList.begin(); it != target.serverList.end(); it++)
	{
		for (std::vector<MultiTree*>::iterator treeIt = (*it)->root.begin(); treeIt != (*it)->root.end(); treeIt++)
		{
			delete (*treeIt);
		}
		delete (*it);
	}
	delete &target;
}

int main()
{
	// atexit(leak);
	std::cout << "http block size: " << sizeof(HttpBlock) << std::endl;
	std::cout << "server block size: " << sizeof(ServerBlock) << std::endl;
	std::cout << "location block size: " << sizeof(LocationBlock) << std::endl;
	HttpBlock* config = new HttpBlock;
	InitHttpBlock(*config);
	// ParseFile("nginx.conf", *config);
	if (ParseFile(CONF_FILE_PATH, *config))
	{
		deleteHttpBlock(*config);
		return 1;
	}
	// PrintTreeStructure(config->root.at(0)->GetRoot());
	// PrintTreeStructure(config->root.at(1)->GetRoot());
	// PrintTreeStructure(config->serverList.at(0)->root.at(0)->GetRoot());
	// PrintTreeStructure(config->serverList.at(1)->root.at(0)->GetRoot());
	// printSearchedResult(*config->root.at(0), "/404.html/");
	// printSearchedResult(*config->root.at(0), "/404.html/src");
	// printSearchedResult(*config->root.at(0), "/404.html/src/something");
	// printSearchedResult(*config->root.at(1), "/50x.html/");
	// printSearchedResult(*config->root.at(1), "/50x.html/something");
	// printSearchedResult(*config->serverList.at(0)->root.at(0), "/1/old-url/kapouetttt/something");
	// printSearchedResult(*config->serverList.at(0)->root.at(0), "/1/old-url/kapouetttt/kapouet/something");
	// printSearchedResult(*config->serverList.at(1)->root.at(0), "/2/2/something");
	// printSearchedResult(*config->serverList.at(1)->root.at(0), "/2/hello/something");
	// printSearchedResult(*config->root.at(0), "src");
	// printSearchedResult(*config->serverList.at(0)->root.at(1), "/1/");
	// // printSearchedResult(*config->serverList.at(1)->root.at(1), "/hello/");
	// printSearchedResult(*config->root.at(1), "/50x.html/");

	// LocationBlock *rootData = new LocationBlock;
	// rootData->uri = "/";
	// MultiTreeNode *temp = new MultiTreeNode(rootData);
	// MultiTree root(*temp);

	// 경로 앞뒤의 '/'처리 결정해야 함
	// addChildURI(root.GetRoot(), "home");
	// addChildURI(root.GetRoot(), "etc");
	// addChildURI(root.GetRoot(), "/");
	// addChildURI(root.searchNodeOrNull("/home/"), "src/");
	// addChildURI(root.searchNodeOrNull("/home/"), "usr/");
	// addChildURI(root.searchNodeOrNull("/home/src/"), "http/");
	// addChildURI(root.searchNodeOrNull("/home/src/http/"), "html/");
	// addChildURI(root.searchNodeOrNull("/home/src/http/"), "css/");
	// addChildURI(root.searchNodeOrNull("/home/src/"), "http/html");
	// addChildURI(root.searchNodeOrNull("/home/src/"), "ftp/");
	// addChildURI(root.searchNodeOrNull("/home/src/"), "image");
	// addChildURI(root.searchNodeOrNull("/home/src/image/"), "png/");
	// addChildURI(root.searchNodeOrNull("/home/src/image/"), "jpg/");

	// // std::cout << Colors::BoldWhiteString("\n[Let's get it on]") << std::endl;
	// printSearchedResult(root, "/home/src/http/html/index.html");
	// printSearchedResult(root, "/home/src/http/http.conf");
	// printSearchedResult(root, "/home/src/http/css/sstyle.cs");
	// printSearchedResult(root, "/etc/something");
	// printSearchedResult(root, "\'nothing\'");
	// printSearchedResult(root, "/something");
	deleteHttpBlock(*config);
	return (0);
}