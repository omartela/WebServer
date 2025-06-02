
#pragma once

#include "Enums.hpp"
#include "Parser.hpp"
#include <string>
#include <map>

class HTTPRequest
{
    private:
        void parser(std::string headers, ServerConfig server);

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
        bool FileUsed;
        int FileFd;
        bool FileIsOpen;
        bool isCGI;
        HTTPRequest();
        HTTPRequest(std::string headers, ServerConfig server);
};