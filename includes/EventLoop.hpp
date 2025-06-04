#pragma once

// #include "timeout.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "CGIhandler.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <map>
#include <queue>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>


#define MAX_CONNECTIONS 1000
#define TIMEOUT 60 //testing only, increase later to 60
#define CHILD_CHECK 1

class EventLoop
{
    public:
        int timerFD;
        int nChildren;
        int childTimerFD;
        int loop;
        bool timerOn;
        std::map<int, ServerConfig> servers;
        std::map<int, Client> clients;
        int serverSocket;
        struct epoll_event setup;
        std::vector<epoll_event> eventLog;
        struct itimerspec timerValues;
        // int loop = epoll_create1(0);

        EventLoop(std::vector<ServerConfig> serverConfigs);
        void startLoop();
        void setTimerValues(int n);
        void checkTimeouts();//int timerFd, std::map<int, Client>& clients, int& children, int loop);
        void closeClient(int fd);//Client& client, std::map<int, Client>& clients, int& children, int loop);
        void createErrorResponse(Client &client, int code, std::string msg, std::string logMsg);
        ~EventLoop();
};
