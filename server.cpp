#include "server.hpp"
#include "Connection.hpp"

void        serverLoop();
static int  initListenerSocket();
static int  acceptNewConnection(int listenerSocket, std::map<int, Connection> connections);
static int  findNewFd(std::map<int, Connection> connections);
static void handleClientRequest();

void serverLoop()
{
    Connection::nConnections = 0;
    Connection::nActiveConnections = 0;
    std::map<int, Connection> connections;
    int nReady;
    int listenerSocket;
    int eventLoop;
    int newConnection;
    struct epoll_event setup;
    std::vector<epoll_event> eventLog(MAX_CONNECTIONS);

    eventLoop = epoll_create(1);
    listenerSocket = initListenerSocket();
    setup.events = EPOLLIN | EPOLLOUT;
    setup.data.fd = listenerSocket;
    if (epoll_ctl(eventLoop, EPOLL_CTL_ADD, listenerSocket, &setup) < 0)
        throw std::runtime_error("listenerSocket epoll_ctl failed");
    while (true)
    {
        nReady = epoll_wait(listenerSocket, eventLog.data(), MAX_CONNECTIONS, -1);
        if (nReady == -1)
            throw std::runtime_error("epoll_wait failed");
        for (int i = 0; i < nReady; i++)
        {
            if (eventLog[i].data.fd == listenerSocket)
            {
                newConnection = acceptNewConnection(listenerSocket, connections);
                setup.data.fd = newConnection;
                if (epoll_ctl(eventLoop, EPOLL_CTL_ADD, newConnection, &setup) < 0)
                    throw std::runtime_error("newConnection epoll_ctl failed");
            }
            else
                handleClientRequest(connections, eventLog[i]);
        }
    }
}

static int initListenerSocket()
{
    int listenerSocket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); //change later to actual value: htons(serverPort)
    bind(listenerSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    listen(listenerSocket, MAX_CONNECTIONS);
    return (listenerSocket);
}

static int acceptNewConnection(int listenerSocket, std::map<int, Connection> connections)
{
    int newFd = findNewFd(connections);
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);

    connections.at(newFd).fd = accept(listenerSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
    return newFd;
}

static int  findNewFd(std::map<int, Connection> connections)
{
    int newFd = -1;

    if (Connection::nConnections < MAX_CONNECTIONS)
    {
        //go thru earlier connections for vacancy
        for (const std::pair<const int, Connection>& oldConnection : connections)
        {
            if (oldConnection.second.state == VACANT)
            {
                newFd = oldConnection.first;
                connections.at(newFd).state = IDLE;
                return newFd;
            }
        }
        //if no vacancy, make new instance
        if (newFd == -1)
        {
            connections.emplace(Connection::nConnections, Connection()); //maybe create a few at a time?
            newFd = Connection::nConnections;
        }
    }
    else
    {
        //newFd = least active connection
        connections.at(newFd).reset();
        //close()?
    }
    return newFd;
}

static void handleClientRequest(std::map<int, Connection> connections, struct epoll_event event)
{
    int fd = event.data.fd;

    switch (connections.at(fd).state)
    {
        case IDLE:
        case RECV_HEADER:
        case RECV_BODY:
        case SEND_HEADER:
        case SEND_BODY:
        case DONE:
        case VACANT:
    }

};