
#pragma once
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"
#include "Client.hpp"

class RequestHandler
{
    public:
        static HTTPResponse handleRequest(Client& client);
        // static HTTPResponse nonMultipart(const HTTPRequest& req);
        static HTTPResponse handleMultipart(Client& client);
    private:
        static HTTPResponse handleGET(Client& client, std::string fullPath);
        static HTTPResponse handlePOST(Client& client, std::string fullPath);
        static HTTPResponse handleDELETE(std::string fullPath);
        static HTTPResponse executeCGI(Client& client, std::string fullPath);
        static bool isAllowedMethod(std::string method, Route route);
};