
#include "../includes/HTTPResponse.hpp"
#include <sstream>

HTTPResponse::HTTPResponse(int code, const std::string& msg) : status(code), stat_msg(msg) {}

std::string HTTPResponse::toString() const
{
    std::ostringstream response;
    response << "HTTP/1.1 " << status << " " << stat_msg << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}