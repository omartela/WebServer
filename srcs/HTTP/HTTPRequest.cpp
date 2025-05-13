
#include "HTTPRequest.hpp"
#include "Enums.hpp"
#include <sstream>
#include <iostream>
#include <vector>

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const std::string raw) { parser(raw); }

/*
    How it works:
    Status line: Includes the HTTP version, status code, and status message (e.g., 200 OK).

    Headers: Loops through all headers and appends them to the response.

    Content-Length: If there's a body, it adds the Content-Length header.

    Body: Appends the body content after the headers.
*/

reqTypes getMethodEnum(const std::string& method)
{
    if (method == "GET") return GET;
    if (method == "POST") return POST;
    if (method == "DELETE") return DELETE;
    return INVALID;
}

void HTTPRequest::parser(const std::string raw)
{
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
            while (!value.empty() && value[0] == ' ')
                value.erase(0, 1);
            headers[key] = value;
        }
    }
    std::map<std::string, std::string>::iterator it = headers.find("Content-Length");
    if (it != headers.end())
    {
        size_t contentLength = std::strtoul(it->second.c_str(), NULL, 10);
        std::vector<char> buffer(contentLength);
        stream.read(buffer.data(), contentLength);
        body.assign(buffer.begin(), buffer.end());
    }
}