
#include "../includes/HTTPResponse.hpp"
#include <sstream>
#include <unordered_map>

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

int HTTPResponse::getStatusCode()
{
    return status;
}

std::string HTTPResponse::getStatusMessage()
{
    return stat_msg;
}

HTTPResponse HTTPResponse::generateErrorResponse(HTTPResponse res)
{
       static std::unordered_map<int, std::string> statusMessages =
       {
        {400, "Bad request"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {503, "Service Unavailable"}
       };

       std::string reason;
       if (statusMessages.find(res.getStatusCode()) != statusMessages.end())
       {
        reason = statusMessages[res.status];
       }
       else
       {
        reason = "Unknown Error";
       }

       HTTPResponse response(res.status, reason);
       std::string body = "<html><head><title>" + std::to_string(res.status) + " " + reason +
                       "</title></head><body><h1>" + std::to_string(res.status) + " " + reason +
                       "</h1><p>The server encountered an error: " + res.getStatusMessage() + ".</p></body></html>";

        response.headers["Content-Type"] = "text/html; charset=UTF-8";
        response.headers["Content-Length"] = std::to_string(body.size());
        response.body = body;

        return response;
}
