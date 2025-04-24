#include "server.hpp"

void reset(connection toReset)
{
    toReset.fd = -1;
    toReset.state = VACANT;
    toReset.read_buffer.clear();
    toReset.write_buffer.clear();
    toReset.bytes_read = 0;
    toReset.bytes_written = 0;
}

int initListenerSocket()
{
    int listenerSocket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);
    sockaddr_in serverAddress;
}

void server_loop()
{
    //struct connection listener;
    std::vector<connection> allConnections(1024);
    int newFd;
    int listenerSocket;
    int eventLoopFd;


    for (connection& oneConnection : allConnections)
        reset(oneConnection);
    eventLoopFd = epoll_create(1);
    listenerSocket = initListenerSocket();
}