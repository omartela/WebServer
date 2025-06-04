#pragma once

// #include "timeout.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "CGIhandler.hpp"
#include "Logger.hpp"
#include <sys/stat.h>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <csignal>
#include <cstdlib>
#include <stdexcept>
#include "Parser.hpp"
#include "Client.hpp"

#define MAX_CONNECTIONS 1000
#define TIMEOUT 60 //testing only, increase later to 60
#define CHILD_CHECK 1

class EventLoop
{
    int timerFD;
    int nChildren;
    int childTimerFD;
    std::map<int, ServerConfig> servers;
    std::map<int, Client> clients;
    int serverSocket;
    struct epoll_event setup;
    std::vector<epoll_event> eventLog(MAX_CONNECTIONS);
    int loop = epoll_create1(0);
    void eventLoop(std::vector<ServerConfig> serverConfigs);
};
