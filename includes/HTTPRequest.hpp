
#pragma once

#include "Enums.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include <string>
#include <map>

class HTTPRequest
{
    private:
        void parser(Client& client);

    public:
        std::string method;
        reqTypes eMethod;
        std::string path;
        std::string file;
        std::string version;
        std::string location;
        std::string query;
        std::string pathInfo;
        std::map<std::string, std::string> headers;
        std::string body;
        HTTPRequest();
        HTTPRequest(Client& client);
};