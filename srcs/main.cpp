#include "Parser.hpp"
#include "Logger.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "RequestHandler.hpp"
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

Logger wslog;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage program name + file" << std::endl;
        return 1;
    }
    try
    {
        Parser parser(argv[1]);
        parser.printServerConfigs();
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return (1);
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "Out of range error: " << e.what() << std::endl;
        return (1);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return (1);
    }
    catch (const std::bad_alloc& e)
    {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
        return (1);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    wslog.writeToLogFile(INFO, "TESTATAAN", true);
    
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
    // wslog.writeToLogFile(INFO, "First line", false);
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