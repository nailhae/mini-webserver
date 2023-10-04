#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <map>
#include <unistd.h>
#include "UserData.hpp"

class Cgi {
    private:
        std::map<std::string, std::string> env;
        char** chEnv;
        char** argv;
        int exitStatus;
        std::string cgiPath;
        pid_t cgiPid;
    public:
        int pipeIn[2];
        int pipeOut[2];

        Cgi();
        Cgi(const std::string& uri);
        ~Cgi();
        Cgi(Cgi const &obj);
        Cgi &operator=(Cgi const &obj);

        void initCgiEnv(std::string httpCgiPath, size_t ContentSize, std::map<int, std::string> Header);
        void initCgiEnv(std::string httpCgiPath, size_t ContentSize, std::map<int, std::string> Header, std::string Body, size_t MethodType);
        void execute(size_t &errorCode);
        // void clear();

        const std::map<std::string, std::string> &getEnv() const;
        const pid_t &getCgiPid() const;
        const std::string &getCgiPath() const;
        void sendCgiBody(std::string reqBody);
        std::string    readCgiResponse();
};

#endif