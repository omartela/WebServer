#pragma once
#include <vector>
#include <string>
#include "Client.hpp"
#include <unistd.h>
#include <sys/wait.h>

class Client;

class CGIHandler
{
    private:
        std::vector<std::string> envVariables;
        char* envArray[16] = {};
        char* exceveArgs[3] = {};
        int writeCGIPipe[2];
        int readCGIPipe[2];
        pid_t childProcPid;
        std::string fullPath;
    public:
        CGIHandler();
        void setEnvValues(Client client);
        HTTPResponse executeCGI(Client& client);
};