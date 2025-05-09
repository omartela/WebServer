#pragma once
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include "Parser.hpp"

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

        size_t currentChunkSize;     // Tämän hetkinen chunkin koko
        std::string chunkBuffer;     // Väliaikainen bufferi chunkin lukemista varten
        std::string bodyBuffer;      // Kokonainen body yhdistettynä chunkista

        Client();
        Client(const Client& copy);
        Client& operator=(const Client& copy);
        ~Client();

        void reset();
        void requestParser();
        void resetRequest();
        bool validateHeader();
};