#include <iostream> 
#include <fstream>
#include <sstream>
#include "../../includes/http/HTTPRequest.hpp"
#include "../../includes/http/HTTPResponse.hpp"
#include "../../includes/http/RequestHandler.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    std::ifstream file(argv[1]);
    if (!file.is_open())
        return 1;
    std::stringstream buff;
    buff << file.rdbuf();
    std::string rawReq = buff.str();
    HTTPRequest req(rawReq);
    HTTPResponse res = RequestHandler::handleRequest(req);
    std::cout << res.toString() << std::endl;
}