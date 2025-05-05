#pragma once
#include <map>
#include "Parser.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

class SocketsHandler
{
    private:
        bool run;
        std::vector<pollfd> fds;
        std::map<int, ServerConfig> serverFDs;
        std::map<int, ServerConfig> clientFDs;
    public:
        SocketsHandler(std::vector<ServerConfig> server_configs);
        void Run();
        void signalHandler(int sig);
        ~SocketsHandler();
};




