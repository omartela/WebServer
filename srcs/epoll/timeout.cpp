#include "timeout.hpp"

void checkTimeouts(int timerFd, std::map<int, Client>& clients)
{

    uint64_t tempBuffer;
    ssize_t bytesRead = read(timerFd, &tempBuffer, sizeof(tempBuffer)); //reading until timerfd event stops
    if (bytesRead != sizeof(tempBuffer))
        throw std::runtime_error("timerfd recv failed");

    if (clients.empty())
    {
        std::cout << "No more clients connected" << std::endl;
        return ;
    }

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    for (auto it = clients.begin(); it != clients.end();)
    {
        auto& client = it->second;

        std::cout << "checking client FD" << client.fd << std::endl;
        std::chrono::steady_clock::time_point timeout = client.timestamp + std::chrono::seconds(10);
        int elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - client.timestamp).count();

        if (now > timeout) //408 request timeout error page?
        {
            std::cout << "client FD" << client.fd << " timed out due to inactivity!" << std::endl;
            // if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
            //     throw std::runtime_error("timeout epoll_ctl DEL failed");
            close(client.fd);
            it = clients.erase(it);
            continue ;
        }

        if ((client.state == READ_HEADER || client.state == READ_BODY) && elapsedTime > 0)
        {
            int dataReceived = client.rawReadData.size() - client.previousDataAmount;
            int dataRate = dataReceived / elapsedTime;
            if ((client.rawReadData.size() > 32 && dataRate < 1024)
                || (client.rawReadData.size() < 32 && dataReceived < 16))
            {
                std::cout << "client FD" << client.fd << " disconnected, client sent data too slowly!" << std::endl;
                // if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
                //     throw std::runtime_error("timeout epoll_ctl DEL failed");
                close(client.fd);
                it = clients.erase(it);
                continue ;
            }
        }

        if (client.state == READ_HEADER && client.rawReadData.size() > 8192) //413 entity too large error page? also replace magic number
        {
            std::cout << "client FD" << client.fd << " disconnected, header size too big!" << std::endl;
            // if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
            //     throw std::runtime_error("timeout epoll_ctl DEL failed");
            close(client.fd);
            it = clients.erase(it);
            continue ;
        }

        if (client.state == READ_BODY && client.rawReadData.size() > client.serverInfo.client_max_body_size) //413 entity too large?
        {
            std::cout << "client FD" << client.fd << " disconnected, body size too big!" << std::endl;
            // if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
            //     throw std::runtime_error("timeout epoll_ctl DEL failed");
            close(client.fd);
            it = clients.erase(it);
            continue ;
        }

        if (client.state == SEND && elapsedTime > 0) //make more comprehensive later
        {
            int dataSent = client.previousDataAmount - client.writeBuffer.size();
            int dataRate = dataSent / elapsedTime;
            if (client.writeBuffer.size() > 1024 && dataRate < 1024) //what is proper amount?
            {
                std::cout << "client FD" << client.fd << " disconnected, client received data too slowly!" << std::endl;
                // if (epoll_ctl(loop, EPOLL_CTL_DEL, client.fd, nullptr) < 0)
                //     throw std::runtime_error("timeout epoll_ctl DEL failed");
                close(client.fd);
                it = clients.erase(it);
                continue ;
            }
        }

        else
        {
            if (client.state == READ_HEADER || client.state == READ_BODY)
                client.previousDataAmount = client.rawReadData.size();
            else if (client.state == SEND)
                client.previousDataAmount = client.writeBuffer.size();
            ++it;
        }
    }
}
