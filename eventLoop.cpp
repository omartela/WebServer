#include "eventLoop.hpp"
#include "Connection.hpp"
#include "Server.hpp"

void        serverLoop(std::vector<ServerConfig> servers);
static int  initServerSocket();
static int  acceptNewConnection(int listenerSocket, std::map<int, Connection> connections);
static int  findNewFd(std::map<int, Connection> connections);
static void handleClientRequest();

void serverLoop(std::vector<ServerConfig> serverConfigs)
{
    //Connection::nActiveConnections = 0;
    std::map<int, Server> servers;
    std::map<int, Connection> connections;
    int serverSocket;
    struct epoll_event setup;
    std::vector<epoll_event> eventLog(MAX_CONNECTIONS);

    int eventLoop = epoll_create1(0);
    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        Server newServer;

        serverSocket = initServerSocket(serverConfigs[i]);
        newServer.fd = serverSocket;

        setup.data.fd = newServer.fd;
        setup.events = EPOLLIN;

        servers[serverSocket] = newServer;

        if (epoll_ctl(eventLoop, EPOLL_CTL_ADD, servers.at(i).fd, &setup) < 0)
            throw std::runtime_error("serverSocket epoll_ctl failed");
    }
    Connection::nextFreeSocketIndex = servers.size() + 3;
    Connection::nConnections = 0;
    while (true)
    {
        int nReady = epoll_wait(eventLoop, eventLog.data(), MAX_CONNECTIONS, -1);
        if (nReady == -1)
            throw std::runtime_error("epoll_wait failed");
            
        for (int i = 0; i < nReady; i++)
        {
            if (servers.find(eventLog[i].data.fd) != servers.end())
            {
                int newConnection = acceptNewConnection(serverSocket, connections);
                setup.events = EPOLLIN | EPOLLOUT;
                setup.data.fd = newConnection;
                if (epoll_ctl(eventLoop, EPOLL_CTL_ADD, newConnection, &setup) < 0)
                    throw std::runtime_error("newConnection epoll_ctl failed");
            }
            else
                handleClientRequest(connections, eventLog[i]);
        }
    }
}

static int initServerSocket(ServerConfig server)
{
    int listenerSocket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(server.port);

    bind(listenerSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    listen(listenerSocket, SOMAXCONN);
    
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

    if (Connection::nextFreeSocketIndex < MAX_CONNECTIONS)
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
            newFd = Connection::nextFreeSocketIndex;
            connections[newFd] = Connection();
            Connection::nextFreeSocketIndex++;
        }
    }
    else
    {
        //newFd = least active connection
        connections.at(newFd).hardReset();
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
            connections.at(fd).state = READ_HEADER;
        case READ_HEADER:
        {
            connections.at(fd).bytesRead = recv(fd, connections.at(fd).readBuffer.data(), sizeof(connections.at(fd).readBuffer), 0); 
            //size limit?
            if (connections.at(fd).bytesRead < 0)
                throw std::runtime_error("recv failed"); //more comprehensive later
            
        }
        case READ_BODY:
        {

        }
        case SEND_HEADER:
        case SEND_BODY:
        case DONE:
            connections.at(fd).softReset();
    }
};