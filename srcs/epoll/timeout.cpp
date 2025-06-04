#include "timeout.hpp"
#include "Logger.hpp"
#include "Client.hpp"
#include "utils.hpp"

void closeClient(Client& client, std::map<int, Client>& clients, int& children, int loop)
{
    if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
        throw std::runtime_error("timeout epoll_ctl DEL failed in closeClient");
    if (client.request.isCGI == true)
        children--;
    close(client.fd);
    clients.erase(client.fd);
    wslog.writeToLogFile(INFO, "Client FD" + std::to_string(client.fd) + " closed!", true);
}

// void checkClosedClients(std::map<int, Client>& clients, int loop, int& children) //not sure if works, havent tested
// {
//     wslog.writeToLogFile(INFO, "Checking closed clients...", true);
//     for (auto it = clients.begin(); it != clients.end();)
//     {
//         auto& client = it->second;
//         ++it;

//         char buf1[1];
//         int readAttempt = recv(client.fd, buf1, sizeof(buf1), MSG_PEEK | MSG_DONTWAIT);
//         wslog.writeToLogFile(INFO, "Client FD" + std::to_string(client.fd) + ", it read this many bytes: " + std::to_string(readAttempt) + ", errno = " + std::to_string(errno), true);
//         if (readAttempt <= 0)
//         {
//             client.response.push_back( HTTPResponse(500, "Internal Server Error"));
//             client.writeBuffer = client.response.back().body;
//             client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT);
//             closeClient(client, clients, children, loop);
//         }
//     }
//     if (signum != 0)
//         signum = 0;
// }

void checkChildrenStatus(int timerFd, std::map<int, Client>& clients, int loop, int& children)
{
    uint64_t tempBuffer;
    ssize_t bytesRead = read(timerFd, &tempBuffer, sizeof(tempBuffer)); //reading until childtimerfd event stops
    if (bytesRead != sizeof(tempBuffer))
        throw std::runtime_error("childTimerFd recv failed");
    
    for (auto it = clients.begin(); it != clients.end(); ++it)
    {
        auto& client = it->second;
        if (children > 0 && client.request.isCGI == true)
        {
            handleClientRecv(client, loop);
            continue ;
        }
    }
}

void checkTimeouts(int timerFd, std::map<int, Client>& clients, int& children, int loop)
{
    uint64_t tempBuffer;
    ssize_t bytesRead = read(timerFd, &tempBuffer, sizeof(tempBuffer)); //reading until timerfd event stops
    if (bytesRead != sizeof(tempBuffer))
        throw std::runtime_error("timerfd recv failed");

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    for (auto it = clients.begin(); it != clients.end();)
    {
        auto& client = it->second;
        ++it;

        int elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - client.timestamp).count();
        std::chrono::steady_clock::time_point timeout = client.timestamp + std::chrono::seconds(TIMEOUT);

        if (now > timeout)
        {
            client.response.push_back(HTTPResponse(408, "Request Timeout"));
            client.writeBuffer = client.response.back().body;
            client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
            wslog.writeToLogFile(INFO, "Client " + std::to_string(client.fd) + " timed out due to inactivity!", true);
            closeClient(client, clients, children, loop);
            continue ;
        }

        if ((client.state == READ_HEADER || client.state == READ_BODY) && elapsedTime > 0)
        {
            int dataReceived = client.rawReadData.size() - client.previousDataAmount;
            int dataRate = dataReceived / elapsedTime;
            if ((client.rawReadData.size() > 64 && dataRate < 1024)
                || (client.rawReadData.size() < 64 && dataReceived < 15))
            {
                client.response.push_back( HTTPResponse(407, "Request Timeout"));
                client.writeBuffer = client.response.back().body;
                client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
                wslog.writeToLogFile(INFO, "Client " + std::to_string(client.fd) + " disconnected, client sent data too slowly!", true);
                closeClient(client, clients, children, loop);
                continue ;
            }
        }

        if (client.state == READ_HEADER && client.rawReadData.size() > 8192) //replace magic number
        {
            client.response.push_back(HTTPResponse(413, "Entity too large"));
            client.writeBuffer = client.response.back().body;
            client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
            wslog.writeToLogFile(INFO, "Client " + std::to_string(client.fd) + " disconnected, header size too big!", true);
            closeClient(client, clients, children, loop);
            continue ;
        }

        if (client.state == READ_BODY && client.rawReadData.size() > client.serverInfo.client_max_body_size)
        {
            client.response.push_back(HTTPResponse(413, "Entity too large"));
            client.writeBuffer = client.response.back().body;
            client.bytesWritten = send(client.fd, client.writeBuffer.data(), client.writeBuffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
            wslog.writeToLogFile(INFO, "Client " + std::to_string(client.fd) + " disconnected, body size too big!", true);
            closeClient(client, clients, children, loop);
            continue ;
        }

        if (client.state == SEND && elapsedTime > 0) //make more comprehensive later
        {
            int dataSent = client.previousDataAmount - client.writeBuffer.size();
            int dataRate = dataSent / elapsedTime;
            if (client.writeBuffer.size() > 1024 && dataRate < 1024) //what is proper amount?
            {
                wslog.writeToLogFile(INFO, "Client " + std::to_string(client.fd) + " disconnected, client received data too slowly!", true);
                closeClient(client, clients, children, loop);
                continue ;
            }
        } 

        // wslog.writeToLogFile(INFO, "Client FD" + std::to_string(client.fd) + " allowed to continue!", true);

        if (client.state == READ_HEADER || client.state == READ_BODY)
            client.previousDataAmount = client.rawReadData.size();
        else if (client.state == SEND)
            client.previousDataAmount = client.writeBuffer.size();
    }
}