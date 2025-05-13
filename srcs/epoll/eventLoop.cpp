#include "eventLoop.hpp"
#include "Client.hpp"
#include "HTTPResponse.hpp"
#include "RequestHandler.hpp"

void        eventLoop(std::vector<ServerConfig> servers);
static int  initServerSocket(ServerConfig server);
static int  acceptNewClient(int loop, int serverSocket, std::map<int, Client>& clients);
static void handleClientRequest(Client& client,  int loop, struct epoll_event& fdLog);
static void handleClientRequestSend(Client& client, int loop, struct epoll_event& fdLog);

void eventLoop(std::vector<ServerConfig> serverConfigs)
{
    //TO DO create timeout
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
        newServer = serverConfigs[i];
        newServer.fd = serverSocket;
        setup.data.fd = newServer.fd;
        setup.events = EPOLLIN;
        if (epoll_ctl(loop, EPOLL_CTL_ADD, serverSocket, &setup) < 0)
            throw std::runtime_error("serverSocket epoll_ctl ADD failed");
        servers[serverSocket] = newServer;
        std::cout << "New server #" << i << " connected, got FD " << newServer.fd << std::endl;
    }
    //createTimerFd();
    while (true)
    {
        int nReady = epoll_wait(loop, eventLog.data(), MAX_CONNECTIONS, -1);
        if (nReady == -1)
            throw std::runtime_error("epoll_wait failed");
            
        for (int i = 0; i < nReady; i++)
        {
            if (servers.find(eventLog[i].data.fd) != servers.end())
            {
                serverSocket = eventLog[i].data.fd;
                Client newClient;
                int clientFd = acceptNewClient(loop, serverSocket, clients);
                setup.data.fd = clientFd;
                setup.events = EPOLLIN | EPOLLOUT;
                newClient.serverInfo = servers[serverSocket];
                newClient.fd = clientFd;
                if (epoll_ctl(loop, EPOLL_CTL_ADD, clientFd, &setup) < 0)
                    throw std::runtime_error("newClient epoll_ctl ADD failed");
                clients[clientFd] = newClient;
                std::cout << "New client with FD "<< clients[clientFd].fd << " connected to server with FD " << serverSocket << std::endl;
            }
            else
            {
                if (eventLog[i].events & EPOLLIN)
                {
                    std::cout << "EPOLLIN" << std::endl;
                    handleClientRequest(clients[eventLog[i].data.fd], loop, eventLog[i]);
                }
                if (eventLog[i].events & EPOLLOUT)
                {
                    std::cout << "EPOLLOUT" << std::endl;
                    handleClientRequestSend(clients[eventLog[i].data.fd], loop, eventLog[i]);
                }
            }
        }
    }
}

/* static void createTimerFd()
{
    int timerFd = timerfd_create();
    



}
 */
static int initServerSocket(ServerConfig server)
{
    int serverSocket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //REMOVE LATER

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(server.port);
    bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    listen(serverSocket, SOMAXCONN);
    
    return (serverSocket);
}

static int acceptNewClient(int loop, int serverSocket, std::map<int, Client>& clients)
{
    //TODO findOldClient(clients);
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);

    int newFd = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
    if (newFd < 0)
    {
        if (errno == EMFILE) //max fds reached
        {
            int oldFd = -1; //findOldClient(clients);
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

static void handleClientRequestSend(Client& client, int loop, struct epoll_event& fdLog)
{
    std::cout << "Request received from client FD " << client.fd << std::endl;
    // exit(0);

    client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT);
    if (client.bytesWritten == 0)
    {
        return ;
    }
    if (client.bytesWritten < 0)
        throw std::runtime_error("header send failed"); //more comprehensive later
    if (static_cast<size_t>(client.bytesWritten) == client.writeBuffer.size())
    {
        std::string cl = client.request.headers["Connection"];
        if (!cl.empty() || client.request.version == "HTTP/1.0")
        {
            if (cl == "close" || cl == "Close" || client.request.version == "HTTP/1.0")
            {
                close(client.fd);
                if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
                    throw std::runtime_error("oldFd epoll_ctl DEL failed");
            }
        }
        else
        {
            std::cout << "NOTE: Exiting as request parsing not ready" << std::endl;
            client.reset();
            fdLog.events &= ~EPOLLOUT;
            epoll_ctl(loop, EPOLL_CTL_MOD, client.fd, &fdLog);

        }
    }
};

static void handleClientRequest(Client& client,  int loop, struct epoll_event& fdLog)
{
    //std::cout << "Request received from client FD " << client.fd << std::endl;
    // std::cout << "NOTE: Exiting as request parsing not ready" << std::endl;
    // exit(0);

    switch (client.state)
    {
        case IDLE:
            client.state = READ_HEADER;

        case READ_HEADER:
        {
            client.bytesRead = 0;
            char buffer[READBUFFERSIZE];
            client.bytesRead = recv(client.fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
            
            if (client.bytesRead < 0)
            {
                //if (errno == EAGAIN || errno == EWOULDBLOCK) //are we allowed to do this?
                    //return ;
                //else
                    throw std::runtime_error("header recv failed"); //more comprehensive later
            }
            buffer[client.bytesRead] = '\0';
            std::string temp(buffer, client.bytesRead);
            client.readBuffer += temp;
            client.readRaw += client.readBuffer;
            if (client.bytesRead >= 4)
            {
                size_t headerEnd = client.readRaw.find("\r\n\r\n");
                if (headerEnd != std::string::npos)
                {
                        client.headerString = client.readRaw.substr(0, headerEnd + 4);
                        client.requestParser();
                        client.bytesRead = 0;
                        client.readRaw = client.readRaw.substr(headerEnd + 4);
                        /// POST request has only body, GET and DELETE do not have body
                        if (client.request.method == "POST")
                            client.state = READ_BODY;
                        else
                        {
                            client.request.body = std::string(client.readBuffer.begin(), client.readBuffer.end());
                            RequestHandler requestHandler;
                            HTTPResponse response = requestHandler.handleRequest(client.request, client.serverInfo);
                            client.writeBuffer = response.toString();
                            return ;
                        }
                }
                else
                    return ;
            }
            else
                return ;
        }
        case READ_BODY:
        {
            if (client.readRaw.size() == stoul(client.request.headers["Content-Length"])) //or end of chunks?
            {
                client.request.body = std::string(client.readBuffer.begin(), client.readBuffer.end());
                RequestHandler requestHandler;
                HTTPResponse response = requestHandler.handleRequest(client.request, client.serverInfo);
                if (response.getStatusCode() >= 400)
                    response = response.generateErrorResponse(response.getStatusCode(), response.getStatusMessage());
                client.writeBuffer = response.toString();
                fdLog.events |= EPOLLOUT;
                fdLog.events &= ~EPOLLIN;
                epoll_ctl(loop, EPOLL_CTL_MOD, client.fd, &fdLog);
                return ;
            }
            else
                return ;
            client.bytesRead = recv(client.fd, client.readBuffer.data(), client.readBuffer.size(), MSG_DONTWAIT);
            if (client.bytesRead < 0)
            {
                //if (errno == EAGAIN || errno == EWOULDBLOCK) //are we allowed to do this?
                    //break ;
                //else
                     throw std::runtime_error("body recv failed"); //more comprehensive later
            }
            client.readRaw += client.readBuffer;
            client.readBuffer[client.bytesRead] = '\0';
        }
    }
};