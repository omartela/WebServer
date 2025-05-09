#pragma once
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include "Parser.hpp"
//#include "HTTPResponse.hpp"

struct httpRequest
{
    std::string method;
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
    SEND_HEADER,
    SEND_BODY,
    DONE
};

class Client {
    public:  //change all these to private? fix later
        int fd;
        enum connectionStates state;
        size_t timeConnected;
        
        std::vector<char> readBuffer;
        std::vector<char> writeBuffer;
        size_t bytesRead;
        size_t bytesWritten;

        ServerConfig serverInfo;

        httpRequest request;

        Client();
        Client(const Client& copy);
        Client& operator=(const Client& copy);
        ~Client();

        void reset();
        void resetRequest();
        void requestParser();
        void removeWhitespaces(std::string& key, std::string& value);
        bool validateHeader();
};