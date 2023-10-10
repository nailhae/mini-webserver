#include "MethodPost.hpp"

#include "Colors.hpp"

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

int MethodPost::GenerateResponse(std::string& uri, LocationBlock& setting, std::map<int, std::string>& headers,
								 std::string& body)
{
	size_t size = 0;

	(void)setting;
	if (headers[TRANSFER_ENCODING] == "chunked")
	{
		headers[CONTENT_LENGTH] = std::to_string(body.size());
		size = strtol(headers[CONTENT_LENGTH].c_str(), NULL, 10);
	}
	std::cout << Colors::BoldBlue << "size: " << body.size() << Colors::Reset << std::endl;

	initCgiEnv(uri, size, headers, body);
	// std::cout << Colors::Red << "[123]" << mResponse << Colors::Reset << '\n';
	if (execute() == ERROR)
		return (0);
	// std::cout << Colors::Red << "[1234]" << mResponse << Colors::Reset << '\n';
	if (sendCgiBody(body) == ERROR)
		return (0);
	// std::cout << Colors::Red << "[12345]" << mResponse << Colors::Reset << '\n';
	readCgiResponse();
	// std::cout << Colors::Red << "[12345]" << mResponse << Colors::Reset << '\n';

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
	// std::cout << "[body]\n" << body << std::endl;
	if (httpCgiPath.at(0) == '/')
	{
		httpCgiPath.erase(0, 1);
	}
	// for (size_t i = 0; i < Body.size(); ++i)
	// {
	// 	std::cout << Body[i];
	// }
	// std::cout << '\n';
	// std::cout << " " << urlEncode(bodyStr) << std::endl;
	// std::cout << " " << bodyStr << std::endl;
	this->env["AUTH_TYPE"] = "BASIC";
	// TODO chunked인 경우 body.size로 받아야 함.
	// this->env["CONTENT_LENGTH"] = std::to_string(body.size());
	this->env["CONTENT_LENGTH"] = std::to_string(ContentSize);
	// this->env["CONTENT_TYPE"] = Header[CONTENT_TYPE];
	this->env["CONTENT_TYPE"] = Header[CONTENT_TYPE];
	// this->env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	// this->env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	std::cout << Colors::Blue << "[123]" << Header[CONTENT_TYPE] << Colors::Reset << '\n';
	this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
	this->env["PATH_INFO"] = httpCgiPath;
	// this->env["PATH_INFO"] = "/";
	this->env["PATH_TRANSLATED"] = this->env["PATH_INFO"];
	// this->env["PATH_TRANSLATED"] = "/";
	// std::string body;
	// body.assign(Body.begin(), Body.end());
	size_t p = Header[CONTENT_TYPE].find("multipart");
	if (p != std::string::npos)
	{
		this->env["QUERY_STRING"] = body;
	}
	// this->env["QUERY_STRING"] = body;
	// std::cout << Colors::BoldRed << body << Colors::Reset << '\n';
	this->env["REMOTE_ADDR"] = Header[HOST];
	// this->env["REMOTE_HOST"]
	// this->env["REMOTE_USER"]
	this->env["REQUEST_METHOD"] = "POST";
	this->env["SCRIPT_NAME"] = httpCgiPath;
	size_t pos = findHostNamePos(Header[HOST], ":");
	this->env["SERVER_NAME"] = (pos > 0 ? Header[HOST].substr(0, pos) : "");
	this->env["SERVER_PORT"] = (pos > 0 ? Header[HOST].substr(pos + 1, Header[HOST].size()) : "");
	this->env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->env["SERVER_SOFTWARE"] = "42webserv";
	// this->env["HTTP_COOKIE"] 헤더의 쿠키값
	// this->env["WEBTOP_USER"] 로그인한 사용자 이름
	// this->env["NCHOME"]

	this->chEnv = (char**)calloc(sizeof(char*), this->env.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->env.begin();
	for (int i = 0; it != this->env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->chEnv[i] = strdup(tmp.c_str());
		std::cout << tmp << std::endl;
	}
	this->argv = (char**)malloc(sizeof(char*) * 3);
	// this->argv[0] = strdup(httpCgiPath.c_str());
	// this->argv[0] = strdup("/usr/bin/python3");
	// this->argv[0] = strdup("/bin/bash");
	// this->argv[0] = strdup("./cgi_tester");
	this->argv[0] = strdup(httpCgiPath.c_str());
	// this->argv[1] = NULL;
	// this->argv[1] = strdup("./");
	this->argv[1] = NULL;
	return;
}

int MethodPost::execute()
{
	if (this->argv[0] == NULL)
	{
		GenerateErrorResponse(500);
		return (ERROR);
	}
	if (pipe(pipeIn) < 0)
	{
		GenerateErrorResponse(500);
		return (ERROR);
	}
	if (pipe(pipeOut) < 0)
	{
		close(pipeIn[0]);
		close(pipeIn[1]);
		GenerateErrorResponse(500);
		return (ERROR);
	}
	std::cout << Colors::BoldBlue << "size: " << mResponse.size() << Colors::Reset << std::endl;
	this->cgiPid = fork();
	if (this->cgiPid == 0)
	{
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		size_t exitStatus = execve(this->argv[0], this->argv, this->chEnv);
		// perror("execve failed");
		exit(exitStatus);
	}
	// else
	// {
	// 	GenerateErrorResponse(500);
	// }
	return (0);
}

int MethodPost::sendCgiBody(std::string& reqBody)
{
	int len = 0;
	int remainLen = reqBody.size();
	std::string::iterator pos = reqBody.begin();
	std::string temp;

	while (pos != reqBody.end())
	{
		std::cout << "body: " << reqBody.size() << " remain size: " << remainLen << std::endl;
		if (remainLen >= BUFFER_SIZE)
		{
			len = write(pipeIn[1], reqBody.c_str(), BUFFER_SIZE);
		}
		else
		{
			len = write(pipeIn[1], reqBody.c_str(), remainLen);
		}
		if (len < 0)
		{
			std::cout << "출력 실패" << std::endl;
			GenerateErrorResponse(500);
			close(pipeIn[1]);
			close(pipeOut[1]);
			return (ERROR);
		}
		else
		{
			std::cout << "read len: " << len << std::endl;
			temp.assign(pos, pos + (len));
			pos += len;
			remainLen -= len;
		}
	}
	close(pipeIn[1]);
	close(pipeOut[1]);
	return (0);
}

void MethodPost::readCgiResponse()
{
	char buffer[BUFFER_SIZE];
	int readBytes = 0;
	int status;

	while (1)
	{
		readBytes = read(pipeOut[0], buffer, BUFFER_SIZE);
		if (readBytes == 0)
		{
			close(pipeIn[0]);
			close(pipeOut[0]);
			waitpid(getCgiPid(), &status, 0);
			if (WEXITSTATUS(status) != 0)
			{
				GenerateErrorResponse(500);
				return;
			}
			else
			{
				break;
			}
		}
		if (readBytes < 0)
		{
			close(pipeIn[0]);
			close(pipeIn[1]);
			close(pipeOut[0]);
			GenerateErrorResponse(500);
			return;
		}
		else
		{
			mResponse.append(buffer, readBytes);
			memset(buffer, 0, sizeof(buffer));
		}
	}
	mResponse.insert(0, "HTTP/1.1 200 OK\r\n");
}
