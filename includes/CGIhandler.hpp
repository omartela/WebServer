#pragma once
#include <vector>
#include <string>
#include "Client.hpp"
#include <unistd.h>
#include <sys/wait.h>

class CGIHandler
{
    private:
        std::vector<std::string> envVariables;
        char* envArray[16] = {};
        int inPipe;
        int outPipe;
        pid_t childProcPid;
    public:
        CGIHandler();
        void setEnvValues(Client client);
};