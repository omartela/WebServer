#pragma once

#include "Parser.hpp"
#include "Enums.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "CGIHandler.hpp"
#include <netinet/in.h>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/stat.h>

#define READ_BUFFER_SIZE 8192
#define BODY_MEMORY_LIMIT 200

enum connectionStates {
    IDLE,
    READ,
    HANDLE_CGI,
    SEND
};


class Client {
    public:  //change all these to private? fix later
        int fd;
        std::chrono::steady_clock::time_point timestamp;
        enum connectionStates state;

        std::string headerString;
        std::string rawReadData;
        size_t previousDataAmount;;
        std::string readBuffer;
        std::string writeBuffer;
        int bytesRead;
        int bytesWritten;
        bool erase;
        size_t bytesSent;
        ServerConfig serverInfo;
        size_t chunkBodySize;

        HTTPRequest                     request;
        std::vector<HTTPResponse>       response;
        CGIHandler                      CGI;
        std::string chunkBuffer;
        
        int childTimerFd;

        Client(int loop, int serverSocket, std::map<int, Client>& clients, ServerConfig server);
        Client(const Client& copy);
        Client& operator=(const Client& copy);
        ~Client();

        void reset();
};