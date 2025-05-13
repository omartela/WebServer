#pragma once
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include "Parser.hpp"
#include "Enums.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"

#define READBUFFERSIZE 1000
// struct httpRequest
// {
//     std::string method;
//     reqTypes    eMethod;
//     std::string path;
//     std::string version;
//     std::map<std::string, std::string> headers;
//     std::string body;
//     size_t contentLen;
// };

enum connectionStates {
    IDLE,
    READ_HEADER,
    READ_BODY,
    READY_TO_SEND
};

class Client {
    public:  //change all these to private? fix later
        int fd;
        size_t timeConnected;
        enum connectionStates state;

        std::string headerString;
        std::string readRaw;
        std::string readBuffer;
        std::string rawRequest;
        std::string writeBuffer;
        int bytesRead;
        int bytesWritten;

        ServerConfig serverInfo;

        HTTPRequest request;
        HTTPResponse response;

        size_t currentChunkSize;     // Tämän hetkinen chunkin koko
        std::string chunkBuffer;     // Väliaikainen bufferi chunkin lukemista varten
        std::string bodyBuffer;      // Kokonainen body yhdistettynä chunkista

        Client();
        Client(const Client& copy);
        Client& operator=(const Client& copy);
        ~Client();

        void reset();
        void resetRequest();
        void requestParser();
        void removeWhitespaces(std::string& key, std::string& value);
        void validateHeader();
        reqTypes getMethodEnum();
};