#include "Colors.hpp"
#include "MultiTree.hpp"

void leak(void)
{
	system("leaks $PPID");
}

static void addChildURI(MultiTreeNode* nodeOrNull, std::string uri)
{
	if (nodeOrNull == NULL)
	{
		std::cout << Colors::RedString("node is NULL!") << std::endl;
		return ;
	}
	locationBlock *data = new locationBlock;
	data->uri = uri;
	nodeOrNull->AddChildNode(data);
	std::cout << Colors::Green << "complete add " << uri << Colors::Reset << std::endl;
}

void printSearchedResult(MultiTree& root, std::string uri)
{
	MultiTreeNode *temp;

	temp = root.searchNodeOrNull(uri);
	if (temp == NULL)
		std::cout << Colors::Red << "failed to find " << uri << Colors::Reset << std::endl;
	else
		std::cout << Colors::Cyan << "found node's URI: " << temp->GetURI() << Colors::Reset << std::endl;
}

int main()
{
	atexit(leak);
	locationBlock *rootData = new locationBlock;
	rootData->uri = "/";
	MultiTreeNode *temp = new MultiTreeNode(rootData);
	MultiTree root(*temp);

	// 경로 앞뒤의 '/'처리 결정해야 함
	addChildURI(root.GetRoot(), "home");
	addChildURI(root.GetRoot(), "etc");
	addChildURI(root.searchNodeOrNull("/home/"), "src/");
	addChildURI(root.searchNodeOrNull("/home/"), "usr/");
	addChildURI(root.searchNodeOrNull("/home/src/"), "http/");
	addChildURI(root.searchNodeOrNull("/home/src/http/"), "html/");
	addChildURI(root.searchNodeOrNull("/home/src/http/"), "css/");
	addChildURI(root.searchNodeOrNull("/home/src/"), "http/html");
	addChildURI(root.searchNodeOrNull("/home/src/"), "ftp/");
	addChildURI(root.searchNodeOrNull("/home/src/"), "image");
	addChildURI(root.searchNodeOrNull("/home/src/image/"), "png/");
	addChildURI(root.searchNodeOrNull("/home/src/image/"), "jpg/");

	std::cout << Colors::BoldWhiteString("\n[Let's get it on]") << std::endl;
	printSearchedResult(root, "/home/src/http/html/index.html");
	printSearchedResult(root, "/home/src/http/http.conf");
	printSearchedResult(root, "/home/src/http/css/style.css");
	printSearchedResult(root, "/etc/something");
	printSearchedResult(root, "\'nothing\'");
	printSearchedResult(root, "/something");

	return (0);
}