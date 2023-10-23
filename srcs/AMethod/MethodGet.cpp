#include "MethodGet.hpp"

#include "ChangeList.hpp"
#include "Colors.hpp"
#include "Error.hpp"
#include "WebServer.hpp"

MethodGet::MethodGet(int fd)
	: AMethod(fd, GET)
{
}

MethodGet::~MethodGet(void)
{
}

static int loadFolderContent(std::string& bodySet, std::string& uri)
{
	DIR* dirInfo = NULL;
	struct dirent* dirEntry = NULL;
	std::stringstream ss;

	dirInfo = opendir(uri.c_str());
	if (dirInfo == NULL)
	{
		Error::Print(uri + " Open Error");
		return (ERROR);
	}
	ss << "<!DOCTYPE HTML>\n<HTML>\n\t<head>\n\t\t<title>Index of " << uri << "</title>\n"
	   << "<style>\n";
	ss << "body {font-family: Arial, sans-serif; background-color: #f4f4f4;margin: 0;padding: 0;}\n";
	ss << "header {background-color: #333;color: #fff;text-align: center;padding: 10px;}\n";
	ss << ".container {max-width: 600px;margin: 0 auto;padding: 20px;background-color: #fff;border-radius: "
		  "5px;box-shadow: 0 0 5px rgba(0, 0, 0, 0.3);}\n";
	ss << "u1 {list-style-type: none;padding: 0;}\n";
	ss << "li {margin-bottom: 10px;}\n";
	ss << ".file {color: #007bff; text-decoration: none;}";
	ss << ".file:hover {background-color: #F0FFF0;text-decoration: underline;}";
	ss << ".dir {color: #B22222; text-decoration: none;}";
	ss << ".dir:hover {background-color: #F0FFF0;text-decoration: underline;}";
	ss << "</style>"
	   << "\t</head>\n\t<body>\n\t\t<header><h1>Index of " << uri
	   << "</h1></header>\n\t\t<div class=\"container\"><u1>";
	while ((dirEntry = readdir(dirInfo)) != NULL)
	{
		if (dirEntry->d_type == DT_DIR)
		{
			dirEntry->d_name[strlen(dirEntry->d_name)] = '/';
			ss << "\t\t\t<li><a class=\"dir\" href=\"" << dirEntry->d_name << "\">";
		}
		else
			ss << "\t\t\t<li><a class=\"file\" href=\"" << dirEntry->d_name << "\">";
		ss << dirEntry->d_name;
		ss << "</br></a></li>\n";
	}
	ss << "\t\t</a></div>\n\t</body>\n</HTML>";
	closedir(dirInfo);
	bodySet = ss.str();
	return (0);
}

static std::string intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

int MethodGet::AutoIndexResponse(std::string& mUri)
{
	std::string body;
	int result = 0;

	result = loadFolderContent(body, mUri);
	if (result == ERROR)
	{
		GenerateErrorResponse(404);
		return (0);
	}
	else
	{
		mResponse = "HTTP/1.1 200 OK\r\nContent-type: ";
		mResponse += "text/html\r\n";
		mResponse += "Content-length: ";
		mResponse += intToString(body.size());
		mResponse += "\r\n\r\n";
		mResponse += body;
	}
	return (0);
}

int MethodGet::GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers,
								std::string& body)
{
	(void)uri;
	(void)setting;
	(void)headers;
	(void)body;
	return (0);
}

int MethodGet::GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders)
{
	std::ifstream requestedFile;

	if (mSetting.bGetMethod == false)
	{
		Error::Print("405 Method Not Allowed");
		GenerateErrorResponse(405);
		return (ERROR);
	}
	if (*(mUri.end() - 1) == '/')
	{
		if (mSetting.autoindex == true)
			AutoIndexResponse(mUri);
		else
		{
			GenerateErrorResponse(403);
		}
	}
	else
	{
		struct stat fileInfo;

		if (stat(mUri.c_str(), &fileInfo) == ERROR)
		{
			Error::Print("Not found: " + mUri);
			GenerateErrorResponse(404);
			return (0);
		}
		else if (S_ISDIR(fileInfo.st_mode) == true)
		{
			mUri += "/" + mSetting.index;
			if (stat(mUri.c_str(), &fileInfo) == ERROR)
			{
				Error::Print("Not found: " + mUri);
				GenerateErrorResponse(404);
				return (0);
			}
		}
		requestedFile.open(mUri, std::ios::binary);
		if (requestedFile.is_open() == false)
		{
			Error::Print("open failed: " + mUri);
			GenerateErrorResponse(404);
			return (0);
		}
		else
		{
			// std::cout << Colors::BlueString("open success: ") << mUri << std::endl;
			GenerateResponseStatusLine(200);
			if (GenerateResponseHeaders(requestedFile, mSetting, mUri, mHeaders) != 304)
			{
				GenerateResponseBody(requestedFile);
			}
			requestedFile.close();
		}
	}
	return (0);
}