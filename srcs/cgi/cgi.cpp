#include "Cgi.hpp"

Cgi::Cgi(): cgiPid(-1), exitStatus(0), cgiPath(""), chEnv(NULL), argv(NULL) {
}

Cgi::Cgi(std::string path): cgiPid(-1), exitStatus(0), cgiPath(path), chEnv(NULL), argv(NULL) {
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

int checkCgiFile(std::string &) {
    
}
//cgi가 있는경우 없는 경우
void Cgi::initCgiEnv(UserData& req, std::string httpCgiPath)
{
    // this->env["AUTH_TYPE"] = "BASIC";
    // this->env["CONTENT_LENGTH"] = iss.str();
    // this->env["CONTENT_TYPE"] = req.getHeader("content-type");
    // this->env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
    // this->env["SCRIPT_NAME"] = cgiExec;
    // this->env["HTTP_ACCEPT"]
    // this->env["HTTP_ACCEPT_CHARSET"]
    // this->env["HTTP_ACCEPT_ENCODING"]
    // this->env["HTTP_ACCEPT_LANGUAGE"]
    // this->env["HTTP_FORWARDED"]
    // this->env["HTTP_HOST"]//필수
    // this->env["HTTP_PROXY_AUTHORIZATION"]
    // this->env["HTTP_USER_AGENT"]
    // this->env["PATH_INFO"]
    // this->env["PATH_TRANSLATED"]
    // this->env["QUERY_STRING"]
    // this->env["REMOTE_ADDR"]
    // this->env["REMOTE_HOST"]
    // this->env["REMOTE_USER"]
    // this->env["REQUEST_METHOD"]
    // this->env["SCRIPT_NAME"]
    // this->env["SERVER_NAME"]
    // this->env["SERVER_PORT"]
    // this->env["SERVER_PROTOCOL"]
    // this->env["SERVER_SOFTWARE"]
    // this->env["HTTP_COOKIE"]
    // this->env["WEBTOP_USER"]
    // this->env["NCHOME"]
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
        
    }
}