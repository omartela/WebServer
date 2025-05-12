
#pragma once
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"
#include "Client.hpp"

class RequestHandler
{
    public:
        static HTTPResponse handleRequest(const httpRequest& req, ServerConfig config);
        static HTTPResponse nonMultipart(const httpRequest& req);
    private:
        static HTTPResponse handleGET(const std::string& path);
        static HTTPResponse handlePOST(const httpRequest& req);
        static HTTPResponse handleDELETE(const std::string& path);
        static HTTPResponse executeCGI(const httpRequest& req);
        static bool isAllowedMethod(std::string method, Route route);
};