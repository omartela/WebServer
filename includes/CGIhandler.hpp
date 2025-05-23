#pragma once
#include <vector>
#include <string>
#include "Client.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>

class Client;

class CGIHandler
{
    private:
        std::vector<std::string> envVariables;
        char* envArray[16] = {};
        char* exceveArgs[3] = {};
        int writeCGIPipe[2]; //inPipe
        int readCGIPipe[2]; //outPipe
        std::string fullPath;
        std::string output;
    public:
        pid_t childPid;
        CGIHandler();
        void setEnvValues(Client client);
        int executeCGI(Client& client);
        void collectCGIOutput(int readFd);
        HTTPResponse generateCGIResponse();
};