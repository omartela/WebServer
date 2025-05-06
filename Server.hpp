#pragma once
#include <string>

class Server
{
    public:
        int fd;
        int port;

        Server();
        Server(const Server& copy);
        Server& operator=(const Server& copy);
        ~Server() { };
};