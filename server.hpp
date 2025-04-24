#pragma once
#include <vector>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>

enum connectionStates {
    VACANT,
    RECV_HEADER,
    RECV_BODY,
    SEND_HEADER,
    SEND_BODY
} e_connectionStates;

struct connection {
    int fd;
    enum connectionStates state;
    std::vector<char> read_buffer;
    std::vector<char> write_buffer;
    size_t bytes_read;
    size_t bytes_written;
} s_connection;