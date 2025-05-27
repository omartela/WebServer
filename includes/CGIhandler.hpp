#pragma once
#include <vector>
#include <string>
#include "Client.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>

// std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);

class Client;

class CGIHandler
{
    private:
        std::vector<std::string> envVariables;
        char* envArray[16] = {};
        char* exceveArgs[3] = {};
        std::string fullPath;
    public:
        std::string output;
        int writeCGIPipe[2]; //inPipe
        int readCGIPipe[2]; //outPipe
        pid_t childPid;
        CGIHandler();
        void setEnvValues(Client client);
        int executeCGI(Client& client);
        HTTPResponse generateCGIResponse();
        bool isFdWritable(int fd);
        bool isFdReadable(int fd); 

        void collectCGIOutput(int readFd);
};