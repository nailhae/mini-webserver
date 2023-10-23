#include "MethodDelete.hpp"

#include <unistd.h>

#include "Error.hpp"

MethodDelete::MethodDelete(int fd)
	: AMethod(fd, DELETE)
{
}

MethodDelete::~MethodDelete(void)
{
}

int MethodDelete::GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers,
								   std::string& body)
{
	(void)uri;
	(void)setting;
	(void)headers;
	(void)body;
	return (0);
}

int MethodDelete::GenerateResponse(std::string& mUri, LocationBlock& mSetting, std::map<int, std::string>& mHeaders)
{
	(void)mHeaders;
	if (mSetting.bDeleteMethod == false)
	{
		Error::Print("405 Method Not Allowed");
		GenerateErrorResponse(405);
		return (ERROR);
	}
	if (access(mUri.c_str(), F_OK) == ERROR)
	{
		Error::Print("404 Not Found");
		GenerateErrorResponse(404);
		return (ERROR);
	}
	if (access(mUri.c_str(), W_OK) == ERROR)
	{
		Error::Print("403 Forbidden");
		GenerateErrorResponse(403);
		return (ERROR);
	}
	if (std::remove(mUri.c_str()) == ERROR)
	{
		Error::Print("500 Internal Server Error");
		GenerateErrorResponse(500);
		return (ERROR);
	}
	std::cout << "Success remove file" << std::endl;
	GenerateResponseStatusLine(200);
	mResponse += "Content-type: text/plain\r\nContent-length: 71\r\nDate: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n";
	mResponse += "<!DOCTYPE HTML><html>  <body>    <h1>File deleted.</h1>  </body></html>";
	return (0);
}