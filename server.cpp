#include "server.hpp"
#include "Connection.hpp"

static int  findNewFd(std::map<int, Connection> connections)
{
    int newFd = -1;

    if (Connection::nConnections < MAX_CONNECTIONS)
    {
        //go thru earlier connections for vacancy
        for (const std::pair<int, Connection>& reusableConnection : connections)
        {
            if (reusableConnection.second.state == VACANT)
            {
                newFd = reusableConnection.first;
                connections.at(newFd).state = IDLE;
                return newFd;
            }
        }
        //if no vacancy, make new instance
        if (newFd == -1)
        {
            connections.emplace(Connection::nConnections, Connection()); //maybe create a few at a time
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

static void acceptNewConnection(int listenerSocket, std::map<int, Connection> connections)
{
    int newFd = findNewFd(connections);
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);

    connections.at(newFd).fd = accept(listenerSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
}

static int initListenerSocket()
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
    Connection::nConnections = 0;
    Connection::nActiveConnections = 0;
    std::map<int, Connection> connections;
    int nReady;
    int listenerSocket;
    int eventLoop;
    struct epoll_event setup;
    std::vector<epoll_event> eventLog(MAX_CONNECTIONS);

    eventLoop = epoll_create(1);
    listenerSocket = initListenerSocket();
    setup.events = EPOLLIN | EPOLLOUT;
    setup.data.fd = listenerSocket;
    epoll_ctl(eventLoop, EPOLL_CTL_ADD, listenerSocket, &setup);
    while (true)
    {
        nReady = epoll_wait(listenerSocket, eventLog.data(), MAX_CONNECTIONS, -1);
        if (nReady == -1)
            std::exit(1); //maybe change later
        for (int i = 0; i < nReady; i++)
        {
            if (eventLog[i].data.fd == listenerSocket)
            {
                acceptNewConnection(listenerSocket, connections);
                epoll_ctl(eventLoop, EPOLL_CTL_ADD, listenerSocket, &setup);
            }
            else
                handleClientRequest()
        }
    }
}