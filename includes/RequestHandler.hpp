
#pragma once
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"

class RequestHandler
{
    public:
        static HTTPResponse handleRequest(HTTPRequest& req);
        // static HTTPResponse nonMultipart(const HTTPRequest& req);
        static HTTPResponse handleMultipart(HTTPRequest& req);
    private:
        static HTTPResponse handleGET(HTTPRequest& req);
        static HTTPResponse handlePOST(HTTPRequest& req);
        static HTTPResponse handleDELETE(const std::string& path);
        static HTTPResponse executeCGI(HTTPRequest& req);
        static bool isAllowedMethod(std::string method, Route route);
};