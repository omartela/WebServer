
#pragma once
// #include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Parser.hpp"
#include "Client.hpp"

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);

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
