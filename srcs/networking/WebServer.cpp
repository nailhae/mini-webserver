#include "WebServer.hpp"

#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "ChangeList.hpp"
#include "Error.hpp"
#include "MultiTree.hpp"
#include "MultiTreeNode.hpp"

void printTreeStructure(MultiTreeNode* node, int depth);

WebServer* WebServer::mWebServer = NULL;

WebServer::WebServer()
{
}

WebServer::WebServer(std::string confFile)
	: mHttp(parseFileOrNull(confFile))
{
	if (mHttp == NULL)
	{
		Error::Print("parse .conf file");
		exit(EXIT_FAILURE);
	}
}

WebServer::~WebServer()
{
}

WebServer* WebServer::GetInstance()
{
	if (mWebServer == NULL)
	{
		mWebServer = new WebServer(CONF_FILE_PATH);
	}
	return mWebServer;
}

void WebServer::DeleteInstance()
{
	mWebServer->deleteHttpBlock(*(mWebServer->mHttp));
	delete mWebServer;
}

void WebServer::deleteHttpBlock(HttpBlock& http)
{
	for (std::vector<ServerBlock*>::iterator it = http.serverList.begin(); it != http.serverList.end(); it++)
	{
		for (std::vector<MultiTree*>::iterator treeIt = (*it)->root.begin(); treeIt != (*it)->root.end(); treeIt++)
		{
			delete (*treeIt);
		}
		delete (*it);
	}
	delete &http;
}

const HttpBlock* WebServer::GetHttp() const
{
	return mWebServer->mHttp;
}

void printTreeStructure(MultiTreeNode* node, int depth = 0)
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
		printTreeStructure((*it), depth + 2); // 들여쓰기 레벨 조절
	}
}