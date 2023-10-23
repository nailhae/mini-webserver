#include "MethodPost.hpp"

#include <fcntl.h>

#include "Colors.hpp"
#include "Error.hpp"
#include "UserData.hpp"
#include "WebServer.hpp"

MethodPost::MethodPost(int type)
	: AMethod(type, POST)
	, chEnv(NULL)
	, argv(NULL)
	, cgiPid(-1)
{
}

int MethodPost::GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers)
{
	(void)uri;
	(void)setting;
	(void)headers;

	return (0);
}

static std::string intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

int MethodPost::GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers,
								 std::string& body)
{
	size_t size = 0;

	(void)setting;
	if (headers[TRANSFER_ENCODING] == "chunked")
	{
		headers[CONTENT_LENGTH] = intToString(body.size());
	}
	size = strtol(headers[CONTENT_LENGTH].c_str(), NULL, 10);
	initCgiEnv(uri, size, headers, body);
	if (execute() == ERROR)
		return (0);
	return (0);
}

MethodPost::~MethodPost()
{
	if (this->chEnv)
	{
		for (int i = 0; this->chEnv[i]; i++)
			free(this->chEnv[i]);
		free(this->chEnv);
	}
	if (this->argv)
	{
		for (int i = 0; this->argv[i]; i++)
			free(argv[i]);
		free(argv);
	}
	this->env.clear();
}

const std::map<std::string, std::string>& MethodPost::getEnv() const
{
	return (this->env);
}

const pid_t& MethodPost::getCgiPid() const
{
	return (this->cgiPid);
}

const std::string& MethodPost::getCgiPath() const
{
	return (this->cgiPath);
}

std::string getPathInfo(std::string& path)
{
	size_t end;
	std::string tmp;

	tmp = path;
	end = tmp.find("?");
	return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

int findHostNamePos(const std::string path, const std::string ch)
{
	if (path.empty())
		return (-1);
	size_t pos = path.find(ch);
	if (pos != std::string::npos)
		return (pos);
	return (-1);
}

void MethodPost::initCgiEnv(std::string httpCgiPath, size_t ContentSize, std::map<int, std::string> Header,
							std::string body)
{
	if (httpCgiPath.at(0) == '/')
	{
		httpCgiPath.erase(0, 1);
	}
	this->env["AUTH_TYPE"] = "BASIC";
	this->env["CONTENT_LENGTH"] = intToString(ContentSize);
	this->env["CONTENT_TYPE"] = Header[CONTENT_TYPE];
	this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
	this->env["PATH_INFO"] = httpCgiPath;
	this->env["PATH_TRANSLATED"] = this->env["PATH_INFO"];
	size_t p = Header[CONTENT_TYPE].find("multipart");
	if (p != std::string::npos)
	{
		this->env["QUERY_STRING"] = body;
	}
	this->env["REMOTE_ADDR"] = Header[HOST];
	this->env["REQUEST_METHOD"] = "POST";
	this->env["SCRIPT_NAME"] = httpCgiPath;
	size_t pos = findHostNamePos(Header[HOST], ":");
	this->env["SERVER_NAME"] = (pos > 0 ? Header[HOST].substr(0, pos) : "");
	this->env["SERVER_PORT"] = (pos > 0 ? Header[HOST].substr(pos + 1, Header[HOST].size()) : "");
	this->env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->env["SERVER_SOFTWARE"] = "42webserv";
	this->env["HTTP_COOKIE"] = Header[CACHE_CONTROL];

	this->chEnv = (char**)calloc(sizeof(char*), this->env.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->env.begin();
	for (int i = 0; it != this->env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->chEnv[i] = strdup(tmp.c_str());
		// std::cout << tmp << std::endl;
	}
	this->argv = (char**)malloc(sizeof(char*) * 3);
	this->argv[0] = strdup("/usr/bin/python3");
	this->argv[1] = strdup(httpCgiPath.c_str());
	this->argv[2] = NULL;
	return;
}

static int setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl");
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

int MethodPost::execute(void)
{
	int sockets[2];

	if (this->argv[0] == NULL)
	{
		GenerateErrorResponse(500);
		return (ERROR);
	}
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
	{
		GenerateErrorResponse(500);
		return (ERROR);
	}
	if (setNonBlocking(sockets[SOCK_PARENT]) == -1)
	{
		GenerateErrorResponse(500);
		return (ERROR);
	}
	mPid = fork();
	if (mPid == -1)
	{
		Error::Print("fork failed");
		GenerateErrorResponse(500);
		return (ERROR);
	}
	else if (mPid == 0)
	{
		close(sockets[SOCK_PARENT]);
		if (dup2(sockets[SOCK_CHILD], STDIN_FILENO) == ERROR || dup2(sockets[SOCK_CHILD], STDOUT_FILENO) == ERROR)
		{
			Error::Print("dup2 failed");
			exit(EXIT_FAILURE);
		}
		close(sockets[SOCK_CHILD]);
		execve(this->argv[0], this->argv, this->chEnv);
		Error::Print("execve failed");
		exit(EXIT_FAILURE);
	}
	else
	{
		close(sockets[SOCK_CHILD]);
		mFd = sockets[SOCK_PARENT];
	}
	return (0);
}
