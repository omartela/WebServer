#include "eventLoop.hpp"
#include "Client.hpp"
#include "HTTPResponse.hpp"
#include "RequestHandler.hpp"

void        eventLoop(std::vector<ServerConfig> servers);
static int  initServerSocket(ServerConfig server);
static int  acceptNewClient(int loop, int serverSocket, std::map<int, Client>& clients);
static void handleClientRequest(Client &client, int loop);
static void handleClientRequestSend(Client &client, int loop);

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
            int fd = eventLog[i].data.fd;
            if (servers.find(fd) != servers.end())
            {
                Client newClient;
                int clientFd = acceptNewClient(loop, fd, clients);
                setup.data.fd = clientFd;
                setup.events = EPOLLIN;
                newClient.serverInfo = servers[fd];
                newClient.fd = clientFd;
                if (epoll_ctl(loop, EPOLL_CTL_ADD, clientFd, &setup) < 0)
                    throw std::runtime_error("newClient epoll_ctl ADD failed");
                clients[clientFd] = newClient;
                std::cout << "New client with FD "<< clients[clientFd].fd << " connected to server with FD " << serverSocket << std::endl;
            }
            else
            {
                Client& client = clients[fd];
                if (eventLog[i].events & EPOLLIN)
                {
                    // std::cout << "EPOLLIN" << std::endl;
                    handleClientRequest(client, loop);
                }
                if (eventLog[i].events & EPOLLOUT)
                {
                    // std::cout << "EPOLLOUT" << std::endl;
                    handleClientRequestSend(client, loop);
                }
                if (client.erase)
                {
                    std::cout << "client erased" << std::endl;
                    clients.erase(client.fd);
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

void toggleEpollEvents(int fd, int loop, uint32_t events) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (events & EPOLLIN)
        events &= ~EPOLLIN;
    else
        events &= ~EPOLLOUT;
    if (epoll_ctl(loop, EPOLL_CTL_MOD, fd, &ev) < 0)
        throw std::runtime_error("epoll_ctl MOD failed");
}

static void handleClientRequestSend(Client &client, int loop)
{
    // std::cout << "Request received from client FD " << client.fd << std::endl;
    if (client.state != READY_TO_SEND) //|| client.writeBuffer.empty())
        return ;
    client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT);
    if (client.bytesWritten == 0)
    {
        close(client.fd);
        client.erase = true;
        epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr);
        return ;
    }
    if (client.bytesWritten < 0) {
        close(client.fd);
        client.erase = true;
        epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr);
        return;
    }
    client.writeBuffer.erase(0, client.bytesWritten);
    if (client.writeBuffer.empty())
    {
        std::string cl = client.request.headers["Connection"];
        if (!cl.empty())
        {
            if (cl == "close" || cl == "Close")
            {
                close(client.fd);
                if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
                    throw std::runtime_error("oldFd epoll_ctl DEL failed");
                client.erase = true;
            }
            else
            {
                std::cout << "We don't close, only toggled." << std::endl;
                client.reset();
                toggleEpollEvents(client.fd, loop, EPOLLIN);
            }
        }
        else if (client.request.version == "HTTP/1.0")
        {
            close(client.fd);
            if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
                throw std::runtime_error("oldFd epoll_ctl DEL failed");
            client.erase = true;
        }
        else
        {
            std::cout << "We don't close, only toggled." << std::endl;
            client.reset();
            toggleEpollEvents(client.fd, loop, EPOLLIN);
        }
    }
};

static void handleClientRequest(Client &client, int loop)
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
                close(client.fd);
                client.erase = true;
                epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr);
                return ;
            }
			if (client.bytesRead == 0)
            {
				// Client disconnected
                std::cout << "ClientFD disconnected " << client.fd << std::endl;
                close(client.fd);
                client.erase = true;
                epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr);
                return ;
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
                        client.headerString[client.headerString.size()] = '\0'; 
                        // client.requestParser();
                        client.request = HTTPRequest(client.headerString);
                        client.bytesRead = 0;
                        client.readRaw = client.readRaw.substr(headerEnd + 4);
                        /// POST request has only body, GET and DELETE do not have body
                        if (client.request.method == "POST")
                            client.state = READ_BODY;
                        else
                        {
                            client.state = READY_TO_SEND;
                            client.request.body = client.readRaw;
                            HTTPResponse response = RequestHandler::handleRequest(client);
                            client.writeBuffer = response.toString();
                            toggleEpollEvents(client.fd, loop, EPOLLOUT);
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
            if (client.readRaw.size() >= stoul(client.request.headers["Content-Length"])) //or end of chunks?
            {
                client.request.body = client.readRaw;
                client.response = RequestHandler::handleRequest(client);
                if (client.response.getStatusCode() >= 400)
                    client.response = client.response.generateErrorResponse(client.response);
                client.writeBuffer = client.response.toString();
                client.state = READY_TO_SEND;
                toggleEpollEvents(client.fd, loop, EPOLLOUT);
                return ;
            }
            client.bytesRead = 0;
            char buffer2[READBUFFERSIZE];
            client.bytesRead = recv(client.fd, buffer2, sizeof(buffer2) - 1, MSG_DONTWAIT);
            if (client.bytesRead < 0)
            {
                //if (errno == EAGAIN || errno == EWOULDBLOCK) //are we allowed to do this?
                    //break ;
                //else
                close(client.fd);
                client.erase = true;
                epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr);
                return ;
            }
            buffer2[client.bytesRead] = '\0';
            std::string temp(buffer2, client.bytesRead);
            client.readBuffer += temp;
            client.readRaw += client.readBuffer;
        }
		case READY_TO_SEND:
			return;
    }
};