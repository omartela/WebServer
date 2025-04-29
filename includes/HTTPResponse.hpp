
#pragma once
#include <string>
#include <map>

class HTTPResponse
{
    private:
        int status;
        std::string stat_msg;
        std::string err_msg;
    public:
        HTTPResponse(int code = 200, const std::string& msg = "OK");
        std::map<std::string, std::string> headers;
        std::string body;
        std::string toString() const;

        // Functions
        int getStatusCode();
        std::string getStatusMessage();
        void setErrMsg(std::string str);
        std::string getErrMsg();
        HTTPResponse generateErrorResponse(int statusCode, std::string errmessage);
};