#include "Server.hpp"

Server::Server() {
    this->fd = -1;
    this->port = -1;
};

Server::Server(const Server& copy)
{
    *this = copy;
};

Server& Server::operator=(const Server& copy)
{
    if (this != &copy)
    {
        this->fd = copy.fd;
        this->port = copy.fd;
    }
    return *this;
};