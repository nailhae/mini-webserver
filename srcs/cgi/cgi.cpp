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

std::string Cgi::getPathInfo(std::string &path) {
    size_t end;
    std::string tmp;

    tmp = path;
    end = tmp.find("?");
    return (end == std::string::npos ? tmp : tmp.substr(0, end))
}

std::string urlEncode(const std::string &path) {
    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); i++) {
        if (std::isalnum(path[i]) || path[i] == '-' || path[i] == '_' || path[i] == '.' || path[i] == '~') {
            oss << path[i];
        } else {
            oss << '%' << std::hex << std::uppercase << int((size_t)path[i]);
        }
    }

}
//cgi가 있는경우 없는 경우
void Cgi::initCgiEnv(UserData& req, std::string httpCgiPath)
{
    this->env["AUTH_TYPE"] = "BASIC";
    this->env["CONTENT_LENGTH"] = req.mContentSize;
    this->env["CONTENT_TYPE"] = req.mContentType;
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";

    this->env["PATH_INFO"] = getPathInfo(this->cgiPath);
    this->env["PATH_TRANSLATED"] = "root 폴더위치" + (this->env["PATH_INFO"]);
    this->env["QUERY_STRING"] = urlEncode(req.QueryString);
    this->env["REMOTE_ADDR"] = "header에 host";
    // this->env["REMOTE_HOST"]
    // this->env["REMOTE_USER"]
    // this->env["REQUEST_METHOD"]
    this->env["SCRIPT_NAME"] = this->cgiPath;
    // this->env["SERVER_NAME"]
    // this->env["SERVER_PORT"]
    // this->env["SERVER_PROTOCOL"]
    // this->env["SERVER_SOFTWARE"]
    // this->env["HTTP_COOKIE"]
    // this->env["WEBTOP_USER"]
    // this->env["NCHOME"]

    this->chEnv = (char **)calloc(sizeof(char *), this->env.size() + 1);
    std::map<std::string, std::string>::const_iterator it = this->env.begin();
    for (int i = 0; it != this->env.end(); it++, i++)
    {
        std::string tmp = it->first + "=" + it->second;
        this->chEnv[i] = strdup(tmp.c_str());
    }
    this->argv = (char **)malloc(sizeof(char *) * 3);
    this->argv[0] = strdup(extPath.c_str());
    this->argv[1] = strdup(this->cgiPath.c_str());
    this->argv[2] = NULL;
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
        exit(this->exitStatus);
    }
    else {
        errorCode = 500;
    }
}