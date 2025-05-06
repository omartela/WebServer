#pragma once
#include <vector>
#include <map>
#include <string>
#include <cstring>

enum connectionStates {
    IDLE,
    READ_HEADER,
    READ_BODY,
    SEND_HEADER,
    SEND_BODY,
    DONE,
    VACANT
} e_connectionStates;

class Connection {
    public:
        //change all these to private? fix later
        int fd;
        enum connectionStates state;
        std::vector<char> readBuffer;
        std::vector<char> writeBuffer;
        size_t bytesRead;
        size_t bytesWritten;
        static int nConnections;
        static int nextFreeSocketIndex;
        //std::vector<int> vacantFds;
        //static int nActiveConnections;

        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;

        Connection();
        Connection(const Connection& copy);
        Connection& operator=(const Connection& copy);
        ~Connection();

        void softReset();
        void hardReset();
};