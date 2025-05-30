#pragma once
#include <vector>
#include <string>
#include "Logger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <fcntl.h>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);

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
        int childPid;
    public:
        // pid_t childPid;
        CGIHandler();
        void    setEnvValues(Client client);
        void    executeCGI(Client& client);
        void    collectCGIOutput(Client& client);
        void    setOutput(std::string newOutput);
        void    writeBodyToChild(Client& client);
        HTTPResponse generateCGIResponse();
        void collectCGIOutput(int readFd);
        int getWritePipe();
        int getChildPid();
};
