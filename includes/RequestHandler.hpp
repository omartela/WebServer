
#pragma once
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"

class RequestHandler
{
    public:
        static HTTPResponse handleRequest(const HTTPRequest& req, ServerConfig config);
        static HTTPResponse nonMultipart(const HTTPRequest& req);
    private:
        static HTTPResponse handleGET(const std::string& path);
        static HTTPResponse handlePOST(const HTTPRequest& req);
        static HTTPResponse handleDELETE(const std::string& path);
        static HTTPResponse executeCGI(const HTTPRequest& req);
        static bool isAllowedMethod(std::string method, ServerConfig config);
};