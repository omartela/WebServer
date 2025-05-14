#pragma once
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
#include "Parser.hpp"
#include "Enums.hpp"
#include "HTTPResponse.hpp"
//#include "HTTPRequest.hpp"
#include "HTTPRequest.hpp"

#define READBUFFERSIZE 1000

struct httpRequest
{
    std::string method;
    reqTypes    eMethod;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t contentLen;
};

enum connectionStates {
    IDLE,
    READ_HEADER,
    READ_BODY,
    TO_SEND
};

class Client {
    public:  //change all these to private? fix later
        int fd;
        std::chrono::steady_clock::time_point timestamp;
        enum connectionStates state;

        std::string headerString;
        std::string rawReadData;
        std::string readBuffer;
        std::string writeBuffer;
        int bytesRead;
        int bytesWritten;
        bool erase;

        ServerConfig serverInfo;

        httpRequest request;
        //HTTPRequest request;
        HTTPRequest request;
        HTTPResponse response;

        Client();
        Client(const Client& copy);
        Client& operator=(const Client& copy);
        ~Client();

        void reset();
<<<<<<< HEAD
        // void requestParser();
=======
        //void resetRequest();
        //void requestParser();
        //void removeWhitespaces(std::string& key, std::string& value);
        //void validateHeader();
        reqTypes getMethodEnum();
>>>>>>> d9af41e (added early time out event in case client sends too much or not enough data between timeouts)
};