
#pragma once

#include "HTTPResponse.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include "Enums.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <map>
#include <vector>
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <dirent.h>

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
