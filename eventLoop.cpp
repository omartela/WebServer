#include "eventLoop.hpp"
#include "Client.hpp"
//#include "Server.hpp"

void        eventLoop(std::vector<ServerConfig> servers);
static int  initServerSocket(ServerConfig server);
static int  acceptNewClient(int loop, int serverSocket, std::map<int, Client>& clients);
static void handleClientRequest(Client client);

void eventLoop(std::vector<ServerConfig> serverConfigs)
{
    //Connection::nActiveConnections = 0;
    std::map<int, ServerConfig> servers;
    std::map<int, Client> clients;
    int serverSocket;
    struct epoll_event setup;
    std::vector<epoll_event> eventLog(MAX_CONNECTIONS);

    int loop = epoll_create1(0);
    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        ServerConfig newServer;
        serverSocket = initServerSocket(serverConfigs[i]);
        newServer.fd = serverSocket;
        setup.data.fd = newServer.fd;
        setup.events = EPOLLIN;
        if (epoll_ctl(loop, EPOLL_CTL_ADD, serverSocket, &setup) < 0)
            throw std::runtime_error("serverSocket epoll_ctl ADD failed");
        servers[serverSocket] = newServer;
    }
    while (true)
    {
        int nReady = epoll_wait(loop, eventLog.data(), MAX_CONNECTIONS, -1);
        if (nReady == -1)
            throw std::runtime_error("epoll_wait failed");
            
        for (int i = 0; i < nReady; i++)
        {
            if (servers.find(eventLog[i].data.fd) != servers.end())
            {
                Client newClient;
                int clientFd = acceptNewClient(loop, serverSocket, clients);
                setup.data.fd = clientFd;
                setup.events = EPOLLIN | EPOLLOUT;
                newClient.serverInfo = servers[serverSocket];
                if (epoll_ctl(loop, EPOLL_CTL_ADD, clientFd, &setup) < 0)
                    throw std::runtime_error("newClient epoll_ctl ADD failed");
                clients[clientFd] = newClient;
            }
            else
                handleClientRequest(clients[eventLog[i].data.fd]);
        }
    }
}

static int initServerSocket(ServerConfig server)
{
    int serverSocket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(server.port);

    bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    listen(serverSocket, SOMAXCONN);
    
    return (serverSocket);
}

static int acceptNewClient(int loop, int serverSocket, std::map<int, Client>& clients)
{
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);

    int newFd = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
    if (newFd < 0)
    {
        if (errno == EMFILE) //max fds reached
        {
            int oldFd = findOldClient(clients); //to do
            if (epoll_ctl(loop, EPOLL_CTL_DEL, oldFd, nullptr) < 0)
                throw std::runtime_error("oldFd epoll_ctl DEL failed");
            close(oldFd);
            clients.at(oldFd).reset();
            newFd = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
        }
        else
            throw std::runtime_error("accept failed");
    }
    return newFd;
}

static void handleClientRequest(Client client)
{
    switch (client.state)
    {
        case IDLE:
            client.state = READ_HEADER;
        case READ_HEADER:
        {
            client.bytesRead = recv(client.fd, client.readBuffer.data(), sizeof(client.readBuffer), 0); 
            //check size limit?
            if (client.bytesRead < 0)
                throw std::runtime_error("recv failed"); //more comprehensive later
            
        }
        case READ_BODY:
        {

        }
        case SEND_HEADER:
        case SEND_BODY:
        case DONE:
            client.reset();
    }
};