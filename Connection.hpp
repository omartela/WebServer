#pragma once
#include <vector>
#include <cstring>

enum connectionStates {
    IDLE,
    RECV_HEADER,
    RECV_BODY,
    SEND_HEADER,
    SEND_BODY,
    DONE,
    VACANT
} e_connectionStates;

class Connection {
    public:
        int fd; //maybe all these variables ought to be private? fix later
        enum connectionStates state;
        std::vector<char> read_buffer;
        std::vector<char> write_buffer;
        size_t bytes_read;
        size_t bytes_written;
        static int nConnections;
        static int nActiveConnections;

        Connection();
        Connection(int newFd);
        Connection(const Connection& copy) = delete;
        Connection& operator=(const Connection& copy) = delete;
        ~Connection() { };
        void reset();
};