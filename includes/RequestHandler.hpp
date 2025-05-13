
#pragma once
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"
#include "Client.hpp"

class RequestHandler
{
    public:
        static HTTPResponse handleRequest(HTTPRequest& req);
        // static HTTPResponse nonMultipart(const HTTPRequest& req);
        static HTTPResponse handleMultipart(HTTPRequest& req);
    private:
        static HTTPResponse handleGET(HTTPRequest& req);
        static HTTPResponse handlePOST(HTTPRequest& req);
        static HTTPResponse handleDELETE(HTTPRequest& req);
        static HTTPResponse executeCGI(HTTPRequest& req);
        static bool isAllowedMethod(std::string method, Route route);
};