
#pragma once
#include <string>
#include <map>

class HTTPResponse
{
    public:
        int status;
        std::string stat_msg;
        std::map<std::string, std::string> headers;
        std::string body;
        HTTPResponse(int code = 200, const std::string& msg = "OK");
        std::string toString() const;
};