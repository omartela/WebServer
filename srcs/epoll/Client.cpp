#include "Client.hpp"

static int findOldestClient(std::map<int, Client>& clients)
{
    return clients.begin()->first;
    int oldestClient = 0;
    std::chrono::steady_clock::time_point oldestTimestamp = std::chrono::steady_clock::now();

    for (auto it : clients)
    {
        if (it.second.timestamp < oldestTimestamp)
        {
            oldestClient = it.second.fd;
            oldestTimestamp = it.second.timestamp;
        }
    }
    return oldestClient;
}

Client::Client(int loop, int serverSocket, std::map<int, Client>& clients, ServerConfig server)
{
    this->state = IDLE;
    this->bytesRead = 0;
    this->bytesWritten = 0;
    this->previousDataAmount = 0;
    this->erase = false;
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    fd = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
    if (fd < 0)
    {
        if (errno == EMFILE) //max fds reached
        {
            int oldFd = findOldestClient(clients);
            //int oldFd = clients.begin()->first;
            if (oldFd != 0)
            {
                if (epoll_ctl(loop, EPOLL_CTL_DEL, oldFd, nullptr) < 0)
                    throw std::runtime_error("oldFd epoll_ctl DEL failed");
                close(oldFd);
                if (clients.find(oldFd) != clients.end())
                    clients.at(oldFd).reset();
                fd = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
            }
        }
        else
            throw std::runtime_error("Accepting new client failed");
    }
    serverInfo = server;
    timestamp = std::chrono::steady_clock::now();
}

Client::~Client()
{
    if (CGI.writeCGIPipe[0] != -1)
        close(CGI.writeCGIPipe[0]);
    if (CGI.writeCGIPipe[1] != -1)
        close(CGI.writeCGIPipe[1]);
    if (CGI.readCGIPipe[0] != -1)
        close(CGI.readCGIPipe[0]);
    if (CGI.readCGIPipe[1] != -1)
        close(CGI.readCGIPipe[1]);
}

Client::Client(const Client& copy)
{
    *this = copy;
}

Client& Client::operator=(const Client& copy)
{
    if (this != & copy)
    {
        this->fd = copy.fd;
        this->state = copy.state;
        this->timestamp = copy.timestamp;
        this->readBuffer = copy.readBuffer;
        this->rawReadData = copy.rawReadData;
        this->previousDataAmount = copy.previousDataAmount;
        this->writeBuffer = copy.writeBuffer;
        this->bytesRead = copy.bytesRead;
        this->bytesWritten = copy.bytesWritten;
        this->serverInfo = copy.serverInfo;
        this->request = copy.request;
    }
    return *this;
}

void Client::reset()
{
    this->state = IDLE;
    this->readBuffer.clear();
    this->chunkBuffer.clear();
    this->rawReadData.clear();
    this->previousDataAmount = 0;
    this->writeBuffer.clear();
    this->headerString.clear();
    this->bytesRead = 0;
    this->bytesWritten = 0;
    this->erase = false;
    this->request = HTTPRequest();
    this->CGI = CGIHandler();
    this->bytesSent = 0;
    this->chunkBodySize = 0;
}
