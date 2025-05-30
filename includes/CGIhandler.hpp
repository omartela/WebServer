#pragma once
#include <vector>
#include <string>
#include "Logger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>

class CGIHandler
{
    private:
        std::vector<std::string> envVariables;
        char* envArray[16] = {};
        char* exceveArgs[3] = {};
    public:
        std::string output;
        int writeCGIPipe[2]; //inPipe
        int readCGIPipe[2]; //outPipe
        pid_t childPid;
        std::string fullPath;
        // pid_t childPid;
        CGIHandler();
        void setEnvValues(HTTPRequest& request, ServerConfig server);
        int executeCGI(HTTPRequest& request, ServerConfig server);
        HTTPResponse generateCGIResponse();
        bool isFdWritable(int fd);
        bool isFdReadable(int fd); 

        void collectCGIOutput(int readFd);
        int getWritePipe();
        int getChildPid();
};
