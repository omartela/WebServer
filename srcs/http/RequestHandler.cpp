
#include "../../includes/http/RequestHandler.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <cstdio>
#include <iostream>


void printRequest(const HTTPRequest &req)
{
    std::cout << req.body << "\n";
    for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << "\r\n";
    };
    std::cout << req.method << "\n" << req.path << std::endl;

}

static std::string getFileExtension(const std::string& path)
{
    size_t dot = path.find_last_of('.');
    return (dot != std::string::npos) ? path.substr(dot) : "";
}

static std::string getMimeType(const std::string& ext)
{
    static std::map<std::string, std::string> types = {
        {".plain", "text/plain"},
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"}
    };
    return types.count(ext) ? types[ext] : "application/octet-stream";
}

static std::string extractFilename(const std::string& part)
{
    size_t start = part.find("filename=\"");
    if (start == std::string::npos)
        return "";
    start += 10;
    size_t end = part.find("\"", start);
    return part.substr(start, end - start);
}

static std::string extractContent(const std::string& part)
{
    size_t start = part.find("\r\n\r\n");
    size_t offset = 4;
    if (start == std::string::npos)
    {
        start = part.find("\n\n");
        offset = 2;
    }
    if (start == std::string::npos)
        return "";
    size_t conStart = start + offset;
    size_t conEnd = part.find_last_not_of("\r\n") + 1;
    if (conEnd <= conStart)
        return "";
    return part.substr(conStart, conEnd - conStart);
}

static std::vector<std::string> split(const std::string& s, const std::string& s2)
{
    std::vector<std::string> result;
    size_t pos = 0;
    while (true)
    {
        size_t start = s.find(s2, pos);
        if (start == std::string::npos)
            break;
        start += s2.length();
        while (start < s.size() && (s[start] == '-' || s[start] == '\r' || s[start] == '\n'))
            start++;
        size_t end = s.find(s2, start);
        std::string part = (end == std::string::npos) ? s.substr(start) : s.substr(start, end - start);
        while (!part.empty() && (part[0] == '\r' || part[0] == '\n'))
            part.erase(0, 1);
        while (!part.empty() && (part.back() == '\r' || part.back() == '\n'))
            part.pop_back();
        if (!part.empty())
            result.push_back(part);
        if (end == std::string::npos)
            break;
        pos = end;
    }
    return result;
}

HTTPResponse RequestHandler::executeCGI(const HTTPRequest& request)
{
    (void)request;
    std::string body = "CGI BODY";
    HTTPResponse res(200, "OK");
    res.body = body;
    res.headers["Content-Length"] = body.size();
    return res;
}

HTTPResponse RequestHandler::handlePOST(const HTTPRequest& req)
{
    printRequest(req);
    if (req.path.find("/cgi-bin/") == 0)
        return executeCGI(req);
    if (req.headers.count("Content-Type") == 0)
        return HTTPResponse(400, "Bad Request 1");
    std::map<std::string, std::string>::const_iterator its = req.headers.find("Content-Type");
    if (its == req.headers.end())
        return HTTPResponse(400, "Bad Request 2");
    std::string ct = its->second;
    std::string boundary;
    std::string::size_type pos = ct.find("boundary=");
    if (pos == std::string::npos)
        return HTTPResponse(400, "Bad Request 3");
    boundary = ct.substr(pos + 9);
    if (!boundary.empty() && boundary[0] == '"')
        boundary = boundary.substr(1, boundary.find('"', 1) - 1);
    std::string bound_mark = "--" + boundary;
    std::vector<std::string> parts = split(req.body, bound_mark);
    for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it)
    {
        std::string& part = *it;
        if (part.empty() || part == "--\r\n" || part == "--")
            continue;
        std::string file = extractFilename(part);
        if (file.empty())
            continue;
        std::string content = extractContent(part);
        std::string folder = req.path;
        if (folder[0] == '/')
            folder.erase(0, 1);
        std::string path = folder + "/" + file;
        std::ofstream out(path.c_str(), std::ios::binary);
        out.write(content.c_str(), content.size());
        out.close();
    }
    HTTPResponse res(200, "OK");
    res.body = "File(s) upploaded successfully\n";
    res.headers["Content-Type"] = "text/plain";
    res.headers["Content-Length"] = std::to_string(res.body.size());
    return res;
}

HTTPResponse RequestHandler::handleGET(const std::string& path)
{
    std::string base_path = "www" + path;
    if (base_path.find("..") != std::string::npos)
        return HTTPResponse(403, "Forbidden");
    struct stat s;
    if (stat(base_path.c_str(), &s) != 0 || access(base_path.c_str(), R_OK) != 0)
        return HTTPResponse(404, "Not Found");
    std::ifstream file(base_path.c_str(), std::ios::binary);
    if (!file.is_open())
        return HTTPResponse(500, "Internal Server Error");
    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    HTTPResponse response(200, "OK");
    response.body = content.str();
    std::string ext = getFileExtension(base_path);
    response.headers["Content-Type"] = getMimeType(ext);
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return response;
}

HTTPResponse RequestHandler::handleDELETE(const std::string& path)
{
    std::string full_path = "." + path;
    if (full_path.find("..") != std::string::npos || full_path.find("/uploads/") == std::string::npos)
        return HTTPResponse(403, "Forbidden");
    if (access(full_path.c_str(), F_OK) != 0)
        return HTTPResponse(404, "Not Found");
    if (remove(full_path.c_str()) != 0)
        return HTTPResponse(500, "Delete Failed");
    HTTPResponse res(200, "OK");
    res.body = "File deleted successfully\n";
    res.headers["Content-Type"] = "text/plain";
    res.headers["Content-Length"] = std::to_string(res.body.size());
    return res;
}

HTTPResponse RequestHandler::handleRequest(const HTTPRequest& req)
{
    if (req.method == "GET")
        return handleGET(req.path);
    else if (req.method == "POST")
        return handlePOST(req);
    else if (req.method == "DELETE")
        return handleDELETE(req.path);
    return HTTPResponse(501, "Not Implemented");
}