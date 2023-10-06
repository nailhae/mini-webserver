#include "cgi.hpp"

Cgi::Cgi(): chEnv(NULL),  argv(NULL), exitStatus(0), cgiPath(""), cgiPid(-1) {
}

Cgi::Cgi(const std::string& uri): chEnv(NULL),  argv(NULL), exitStatus(0), cgiPath(uri), cgiPid(-1) {
}

Cgi::~Cgi() {
    if (this->chEnv)
    {
        for (int i = 0; this->chEnv[i]; i++)
            free(this->chEnv[i]);
        free(this->chEnv);
    }
    if (this->argv) {
        for (int i = 0; this->argv[i]; i++)
            free(argv[i]);
        free(argv);
    }
    this->env.clear();
}

const std::map<std::string, std::string> &Cgi::getEnv() const
{
    return (this->env);
}

const pid_t &Cgi::getCgiPid() const {
    return (this->cgiPid);
}

const std::string &Cgi::getCgiPath() const  {
    return (this->cgiPath);
}

std::string getPathInfo(std::string &path) {
    size_t end;
    std::string tmp;

    tmp = path;
    end = tmp.find("?");
    return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

std::string urlEncode(const std::vector<unsigned char> &path) {
    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); i++) {
        if (std::isalnum(path[i]) || path[i] == '-' || path[i] == '_' || path[i] == '.' || path[i] == '~') {
            oss << path[i];
        } else {
            oss << '%' << std::hex << std::uppercase << int((size_t)path[i]);
        }
    }
    return (oss.str());
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

void Cgi::initCgiEnv(std::string httpCgiPath, size_t ContentSize, std::map<int, std::string> Header, std::vector<unsigned char>& Body)
{
    if (httpCgiPath.at(0) == '/')
    {
        httpCgiPath.erase(0, 1);
    }
	std::cout << "[Path]" << httpCgiPath << "\n[Body]" << std::endl;
    for (std::vector<unsigned char>::iterator it = Body.begin(); it != Body.end(); it++)
    {
        std::cout << *it;
    }
    std::cout << std::endl;
    this->env["AUTH_TYPE"] = "BASIC";
    this->env["CONTENT_LENGTH"] = ContentSize;
    // this->env["CONTENT_TYPE"] = Header[CONTENT_TYPE];
    this->env["CONTENT_TYPE"] = Header[CONTENT_TYPE];
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->env["PATH_INFO"] = httpCgiPath;
    this->env["PATH_TRANSLATED"] = this->env["PATH_INFO"];
    this->env["QUERY_STRING"] = urlEncode(Body);
    this->env["REMOTE_ADDR"] = Header[HOST];
    // this->env["REMOTE_HOST"]
    // this->env["REMOTE_USER"]
    this->env["REQUEST_METHOD"] = POST;
    this->env["SCRIPT_NAME"] = httpCgiPath;
    // this->env["SCRIPT_FILENAME"] = "3OfD_0ON3b5Mw9GmxClakX77pOo2tHJnNugH0kaRM3-yJ6NBID2Xbb-pG9sd0z-RAgBEwBFP1tijbVV5Qe8aFA.webp";
    size_t pos = findHostNamePos(Header[HOST], ":");
    this->env["SERVER_NAME"] = (pos > 0 ? Header[HOST].substr(0, pos) : "");
    this->env["SERVER_PORT"] = (pos > 0 ? Header[HOST].substr(pos + 1, Header[HOST].size()) : "");
    this->env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->env["SERVER_SOFTWARE"] = "42webserv";
    // this->env["HTTP_COOKIE"] 헤더의 쿠키값
    // this->env["WEBTOP_USER"] 로그인한 사용자 이름
    // this->env["NCHOME"]

    this->chEnv = (char **)calloc(sizeof(char *), this->env.size() + 1);
    std::map<std::string, std::string>::const_iterator it = this->env.begin();
    for (int i = 0; it != this->env.end(); it++, i++)
    {
        std::string tmp = it->first + "=" + it->second;
        this->chEnv[i] = strdup(tmp.c_str());
    }
    this->argv = (char **)malloc(sizeof(char *) * 3);
    // this->argv[0] = strdup(httpCgiPath.c_str());
    this->argv[0] = strdup("/usr/bin/python3");
    // this->argv[1] = strdup(this->cgiPath.c_str());
    this->argv[1] = strdup(httpCgiPath.c_str());
    this->argv[2] = NULL;
    return ;
}

void Cgi::execute(size_t &errorCode)
{
    if (this->argv[0] == NULL || this->argv[1] == NULL) {
        errorCode = 500;
        return ;
    }
    if (pipe(pipeIn) < 0)
    {
        errorCode = 500;
        return ;
    }
    if (pipe(pipeOut) < 0)
    {
        close(pipeIn[0]);
        close(pipeIn[1]);
        errorCode = 500;
        return ;
    }
    this->cgiPid = fork();
    if (this->cgiPid == 0)
    {
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeIn[0]);
        close(pipeIn[1]);
        close(pipeOut[0]);
        close(pipeOut[1]);
        this->exitStatus = execve(this->argv[0], this->argv, this->chEnv);
        // perror("execve failed");
        exit(this->exitStatus);
    }
    else {
        errorCode = 500;
    }
    return ;
}

void Cgi::sendCgiBody(std::vector<unsigned char>& mBody)
{
    size_t bodySize;

    while (mBody.size() >= 0)
    {
        if (mBody.size() >= BUFFER_SIZE){
            bodySize = write(pipeIn[1], mBody.data(), BUFFER_SIZE);
        }
        else
            bodySize = write(pipeIn[1], mBody.data(), mBody.size());

        if (bodySize < 0)
        {
            //error(500);
            break;
        }
        else if (bodySize == 0 || bodySize == mBody.size())
        {
            close(pipeIn[1]);
            close(pipeOut[1]);
        	std::cout << "[read]" << std::endl;
            break;
        }
        else
        {
            // mBody = mBody.substr(bodySize);
        }
    }
    return ;
}

std::string    Cgi::readCgiResponse()
{
    std::string responseContent;
    char    buffer[BUFFER_SIZE];
    int     readBytes = 0;
    responseContent.insert(0, "HTTP/1.1 200 OK\r\n");

    readBytes = read(pipeOut[0], buffer, 40000);
    if (readBytes == 0)
    {
        close(pipeIn[0]);
        close(pipeIn[0]);
        int status;
        waitpid(getCgiPid(), &status, 0);
        if (WEXITSTATUS(status) != 0)
        {
            // error(502);
        }
    }
    else if (readBytes < 0)
    {
        close(pipeIn[0]);
        close(pipeOut[0]);
        close(pipeOut[0]);
        // error(500);
    }
    else{
        responseContent.append(buffer, readBytes);
        memset(buffer, 0, sizeof(buffer));
    }
    // while ((readBytes = read(this->pipeOut[0], buffer, BUFFER_SIZE)) >= 0)
    // {
    //     responseContent.append(buffer, readBytes);
    //     memset(buffer, 0, sizeof(buffer));
    // }
    // if (readBytes < 0)
    // {
    //     close(this->pipeIn[0]);
    //     close(this->pipeIn[1]);
    //     close(this->pipeOut[0]);
    //     //error(500);
    //     return;
    // }

    // close(this->pipeIn[0]);
    // close(this->pipeIn[1]);
    
    // int status;
    
    // waitpid(this->getCgiPid(), &status, 0);
    
	// if (WEXITSTATUS(status) != 0)
	// {
	// 	//error(502);
	// }
    return (responseContent);
}
