#include "parser.hpp"
#include "../util/MultiTree.hpp"
#include "../util/MultiTreeNode.hpp"

int main()
{
	HttpBlock *config = new HttpBlock;
	InitHttpBlock(*config);
	// ParseFile("nginx.conf", *config);
	if (ParseFile("default.conf", *config))
		return 1;
	printSearchedResult(*config->root.at(0), "/404.html/");
	printSearchedResult(*config->root.at(0), "/src/");
	printSearchedResult(*config->serverList.at(0)->root.at(0), "/1/");
	printSearchedResult(*config->serverList.at(0)->root.at(1), "/1/");
	// printSearchedResult(*config->serverList.at(1)->root.at(0), "/2");
	// printSearchedResult(*config->serverList.at(1)->root.at(1), "/hello/");
	// printSearchedResult(*config->root.at(1), "/50x.html/");

	// LocationBlock *rootData = new LocationBlock;
	// rootData->uri = "/";
	// MultiTreeNode *temp = new MultiTreeNode(rootData);
	// MultiTree root(*temp);

	
	// // 경로 앞뒤의 '/'처리 결정해야 함
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
	// printSearchedResult(root, "/home/src/http/css/style.css");
	// printSearchedResult(root, "/etc/something");
	// printSearchedResult(root, "\'nothing\'");
	// printSearchedResult(root, "/something");
	return (0);
}