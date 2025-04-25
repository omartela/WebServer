 
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "../../includes/http/HTTPRequest.hpp"
#include "../../includes/http/HTTPResponse.hpp"
#include "../../includes/http/RequestHandler.hpp"


int main()
{
    int servFD = socket(AF_INET, SOCK_STREAM, 0);
    if (servFD == -1)
    {
        perror("socket");
        return 1;
    }
    int opt = 1;
    if (setsockopt(servFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(servFD);
        return 1;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(servFD, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(servFD);
        return 1;
    }
    if (listen(servFD, SOMAXCONN) < 0)
    {
        perror("listen");
        close(servFD);
        return 1;
    }
    std::cout << "Server is running on http://localhost:8080" << std::endl;
    while (true)
    {
        int clientFD = accept(servFD, NULL, NULL);
        if (clientFD < 0)
        {
            perror("accept");
            continue;
        }
        char buffer[8192];
        ssize_t bRead = read(clientFD, buffer, sizeof(buffer));
        if (bRead <= 0)
        {
            close(clientFD);
            continue;
        }
        std::string rawReq(buffer, bRead);
        HTTPRequest req(rawReq);
        HTTPResponse res = RequestHandler::handleRequest(req);
        std::string rawRes = res.toString();
        send(clientFD, rawRes.c_str(), rawRes.size(), 0);
        close(clientFD);
    }
    close(servFD);
    return 0;
}
// if (argc != 2)
//     return 1;
// std::ifstream file(argv[1]);
// if (!file.is_open())
//     return 1;
// std::stringstream buff;
// buff << file.rdbuf();
// std::string rawReq = buff.str();
// HTTPRequest req(rawReq);
// HTTPResponse res = RequestHandler::handleRequest(req);
// std::cout << res.toString() << std::endl;