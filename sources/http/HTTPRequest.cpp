
#include "../../includes/http/HTTPRequest.hpp"
#include <sstream>
#include <iostream>

HTTPRequest::HTTPRequest(const std::string& raw){ parser(raw); }

/*
    How it works:
    Status line: Includes the HTTP version, status code, and status message (e.g., 200 OK).

    Headers: Loops through all headers and appends them to the response.

    Content-Length: If there's a body, it adds the Content-Length header.

    Body: Appends the body content after the headers.
*/
void HTTPRequest::parser(const std::string& raw) 
{
    std::istringstream stream(raw);
    std::string line;
    if (!std::getline(stream, line))
        return ;
    std::istringstream request_line(line);
    request_line >> method >> path;
    while (std::getline(stream, line) && line != "\r" && !line.empty())
    {
        if (line.back() == '\r')
            line.pop_back();
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
    std::ostringstream body_stream;
    body_stream << stream.rdbuf();
    body = body_stream.str();
}