#pragma once
#include <vector>
#include <cstring>

enum connectionStates {
    VACANT,
    RECV_HEADER,
    RECV_BODY,
    SEND_HEADER,
    SEND_BODY
} e_connectionStates;

class Connection {
    public:
        int fd; //maybe these variables ought to be private?
        enum connectionStates state;
        std::vector<char> read_buffer;
        std::vector<char> write_buffer;
        size_t bytes_read;
        size_t bytes_written;
        static int nConnections;

        Connection();
        Connection(int newFd);
        Connection(const Connection& copy) = delete;
        Connection& operator=(const Connection& copy) = delete;
        ~Connection() { };
        void reset();
};