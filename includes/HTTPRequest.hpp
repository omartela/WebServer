
#pragma once

#include <string>
#include <map>

class HTTPRequest 
{
    private:
        void parser(const std::string  &raw);

    public:
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        HTTPRequest(const std::string& raw);
};