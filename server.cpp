#include "server.hpp"
#include "Connection.hpp"

void acceptNewConnection(std::map<int, Connection> connections)
{
    if (Connection::nConnections < MAX_CONNECTIONS)
    {
        connections.emplace(Connection::nConnections, Connection());
        //accept() new connection
    }
    else
    {
        //find most inactive connection
        int inactiveFd = 0; //replace with actual fd
        connections[inactiveFd].reset();
        //close(inactiveFd);
        connections.emplace(inactiveFd, Connection(inactiveFd));
    }
}

int initListenerSocket()
{
    int listenerSocket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); //change later to actual value: htons(serverPort)
    bind(listenerSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    listen(listenerSocket, MAX_CONNECTIONS);
}

void serverLoop()
{
    std::map<int, Connection> connections;
    int nReady;
    int listenerSocket;
    int eventLoopFd;
    struct epoll_event e;
    std::vector<epoll_event> eventLog(MAX_CONNECTIONS);
    Connection::nConnections = 0;

    eventLoopFd = epoll_create(1);
    listenerSocket = initListenerSocket();
    e.events = EPOLLIN | EPOLLOUT;
    e.data.fd = listenerSocket;
    epoll_ctl(eventLoopFd, EPOLL_CTL_ADD, listenerSocket, &e);
    while (true)
    {
        nReady = epoll_wait(listenerSocket, eventLog.data(), MAX_CONNECTIONS, -1);
        if (nReady == -1)
            std::exit(1); //maybe change later
        for (int i = 0; i < nReady; i++)
        {
            if (eventLog[i].data.fd == listenerSocket)
                acceptNewConnection(connections);
        }
    }
}