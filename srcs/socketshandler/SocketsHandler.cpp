#include "Socketshandler.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "RequestHandler.hpp"

SocketsHandler::SocketsHandler(std::vector<ServerConfig> server_configs)
{
    std::cout << "amount of servers " << server_configs.size() << std::endl;
    for (size_t i = 0; i < server_configs.size(); i++)
    {

        int servFD = socket(AF_INET, SOCK_STREAM, 0);
        if (servFD == -1)
        {
            throw std::runtime_error("socket");
        }
        int opt = 1;
        if (setsockopt(servFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            close(servFD);
            throw std::runtime_error("setsockopt");
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server_configs[i].port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(servFD, (sockaddr*)&addr, sizeof(addr)) < 0)
        {
            close(servFD);
            throw std::runtime_error("bind");
        }
        // Set the socket to non-blocking mode
        if (fcntl(servFD, F_SETFL, O_NONBLOCK) < 0)
        {
            throw std::runtime_error("fcntl");
        }
        if (listen(servFD, SOMAXCONN) < 0)
        {
            close(servFD);
            throw std::runtime_error("listen");
        }

        fds.push_back(pollfd{servFD, POLLIN, 0});
        serverFDs[servFD] = server_configs[i];
    }
    run = true;
}

SocketsHandler::~SocketsHandler()
{
    for (size_t i = 0; i < fds.size(); i++)
    {
        close(fds[i].fd);
    }
}

void SocketsHandler::signalHandler(int sig)
{
    if (sig == SIGINT)
    {
        run = false;
    }
}

void SocketsHandler::Run()
{
    while (run)
    {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0)
        {
            perror("Poll failed");
            return;
        }
        for (size_t i = 0; i < fds.size();)
        {
            /// CHECK OTHER FLAGS FOR ERROR CASES IN REVENTS
            if (fds[i].revents & POLLIN)
            {
                // accept eli new connection
                if (serverFDs.count(fds[i].fd))
                {
                    //accept or functin to do this
                    int clientFD = accept(fds[i].fd, NULL, NULL);
                    if (clientFD < 0)
                    {
                        perror("accept");
                        continue;
                    }
                    else
                    {
                        clientFDs[clientFD] = serverFDs[fds[i].fd];
                        fds.push_back(pollfd{clientFD, POLLIN, 0});
                    }
                    // Set the socket to non-blocking mode
                    if (fcntl(clientFD, F_SETFL, O_NONBLOCK) < 0)
                    {
                        perror("client fd fcntl");
                        continue;
                    }
                }
                else
                {
                    char buffer[8192];
                    ssize_t bRead = read(fds[i].fd, buffer, sizeof(buffer));
                    if (bRead <= 0)
                    {
                        close(fds[i].fd);
                        clientFDs.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        continue;
                    }
                    else
                    {
                        std::string rawReq(buffer, bRead);
                        HTTPRequest req(rawReq, serverFDs[fds[i].fd]);
                        HTTPResponse res = RequestHandler::handleRequest(req);
                        std::string rawRes;
                        if (res.getStatusCode() >= 400)
                        {
                            res = res.generateErrorResponse(res.getStatusCode(), res.getStatusMessage());
                            rawRes = res.toString();
                        }
                        else
                            rawRes = res.toString();
                        send(fds[i].fd, rawRes.c_str(), rawRes.size(), 0);
                        close(fds[i].fd);
                        clientFDs.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                    }
                }
            }
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                close(fds[i].fd);
                clientFDs.erase(fds[i].fd);
                fds.erase(fds.begin() + i);
                continue;
            }
            ++i;
        }
    }
}
