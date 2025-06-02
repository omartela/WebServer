#include "HTTPRequest.hpp"
#include <filesystem>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2) //rename joinPaths
{
    // std::filesystem::path full = path1 / path2;
    // return full.string();
    return path1 / path2;
}

void handleSIGPIPE(int signum)

bool validateHeader(HTTPRequest req)
{
    //check if request line is valid
    if (req.method.empty() || req.path.empty() || req.version.empty())
        return false;

    // if HTTP/1.1 must have host header
    if (req.version == "HTTP/1.1")
    {
        auto it = req.headers.find("Host");
        if (it == req.headers.end())
            return false;
    }
    //if method is POST, check if transfer-encoding exist. if so, it must be chunked and content-length must not exist
    if (req.method == "POST")
    {
        bool transferEncoding = false;
        bool contentLength = false;

        auto it = req.headers.find("Transfer-Encoding");
        if (it != req.headers.end())
        {
            transferEncoding = true;
            if (it->second != "chunked")
                return false;
        }

        it = req.headers.find("Content-Length");
        if (it != req.headers.end())
            contentLength = true;

        if ((transferEncoding == false && contentLength == false)
            || (transferEncoding == true && contentLength == true))
            return false;
    }
    return true;
}