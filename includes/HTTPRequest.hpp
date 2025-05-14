
#pragma once

#include "Enums.hpp"
#include "Parser.hpp"
#include <string>
#include <map>

class HTTPRequest
{
    private:
        void parser(const std::string raw);

    public:
        std::string method;
        reqTypes eMethod;
        std::string path;
        std::string file;
        std::string version;
        std::string location;
        std::map<std::string, std::string> headers;
        std::string body;
        HTTPRequest();
        HTTPRequest(const std::string raw);
};