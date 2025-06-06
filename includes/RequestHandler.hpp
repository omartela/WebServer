
#pragma once
// #include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"


class RequestHandler
{
    public:
        static HTTPResponse handleRequest(Client& client);
        static HTTPResponse handleMultipart(Client& client);
        static bool isAllowedMethod(std::string method, Route route);
    private:
        static HTTPResponse handleGET(Client& client, std::string fullPath);
        static HTTPResponse handlePOST(Client& client, std::string fullPath);
        static HTTPResponse handleDELETE(std::string fullPath);
        static HTTPResponse redirectResponse(std::string fullPath);
};
