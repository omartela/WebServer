#pragma once
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
#include <chrono>
#include "Parser.hpp"
#include "Enums.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "CGIhandler.hpp"

#define READ_BUFFER_SIZE 8192

enum connectionStates {
    IDLE,
    READ_HEADER,
    READ_BODY,
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
        std::string CGIOutput;
        int bytesRead;
        int bytesWritten;
        bool erase;
        ServerConfig serverInfo;

        HTTPRequest                     request;
        std::vector<HTTPResponse>       response;
        CGIHandler                      CGI;
        std::string chunkBuffer;     // Väliaikainen bufferi chunkin lukemista varten
        
        //int childWritePipeFd;
        //int childReadPipeFd;
        //int childPid;
        int childTimerFd;

        Client();
        Client(const Client& copy);
        Client& operator=(const Client& copy);
        ~Client();

        void reset();
};