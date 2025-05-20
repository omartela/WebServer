
#include "HTTPRequest.hpp"
#include "Enums.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(std::string headers, ServerConfig server) { parser(headers, server); }

reqTypes getMethodEnum(const std::string& method)
{
    if (method == "GET") return GET;
    if (method == "POST") return POST;
    if (method == "DELETE") return DELETE;
    return INVALID;
}

void HTTPRequest::parser(std::string raw, ServerConfig server)
{
    isCGI = false;
    std::istringstream stream(raw);
    std::string line;
    if (!std::getline(stream, line))
        return ;
    if (line.back() == '\r')
        line.pop_back();
    std::istringstream request_line(line);
    request_line >> method >> path >> version;
    eMethod = getMethodEnum(method);
    file = path.substr(path.find_last_of("/"));
    while (std::getline(stream, line))
    {
        if (line.back() == '\r')
            line.pop_back();
        if (line.empty())
            break;
        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            key.erase(std::remove_if(key.begin(), key.end(), [](char c){ return (c == ' ' || c == '\n' || c == '\r' ||
                c == '\t' || c == '\v' || c == '\f');}), key.end());

            value.erase(std::remove_if(value.begin(), value.end(), [](char c){ return (c == ' ' || c == '\n' || c == '\r' ||
                    c == '\t' || c == '\v' || c == '\f');}), value.end());
            headers[key] = value;
        }
    }
    location = path.substr(0, path.find_last_of("/") + 1);
    if (server.routes.find(location) != server.routes.end())
    {
        if (!server.routes.at(location).cgiexecutable.empty())
            isCGI = true;
    }
}
