#ifndef CGI_HPP
#define CGI_HPP

#include "../networking/UserData.hpp"
#include <iostream>
#include <map>
#include <unistd.h>

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
        Cgi(std::string path);
        ~Cgi();
        Cgi(Cgi const &obj);
        Cgi &operator=(Cgi const &obj);

        void initCgiEnv(std::string httpCgiPath);
        void execute(size_t &errorCode);
        // void clear();

        const std::map<std::string, std::string> &getEnv() const;
        const pid_t &getCgiPid() const;
        const std::string &getCgiPath() const;
        void sendCgiBody();
        void    readCgiResponse(Client &c, CgiHandler &cgi);
};

#endif