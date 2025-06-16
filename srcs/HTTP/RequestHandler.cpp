
#include "RequestHandler.hpp"

void printRequest(const HTTPRequest &req)
{
    std::cout << req.method << " " << req.path << std::endl;
    for (auto it = req.headers.begin(); it != req.headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << "\r\n";
    };
    std::cout <<"\r\n";
    std::cout << req.body << std::endl;

}

static std::string getMimeType(const std::string& ext)
{
    static std::map<std::string, std::string> types = {
        {".aac", "audio/aac"}, {".abw", "application/x-abiword"},
        {".apng", "image/apng"}, {".arc", "application/x-freearc"},
        {".avif", "image/avif"}, {".avi", "video/x-msvideo"},
        {".azw", "application/vnd.amazon.ebook"}, {".bin", "application/octet-stream"},
        {".bmp", "image/bmp"}, {".bz", "application/x-bzip"},
        {".bz2", "application/x-bzip2"}, {".cda", "application/x-cdf"},
        {".csh", "application/x-csh"}, {".css",	"text/css"},
        {".csv", "text/csv"}, {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".eot", "application/vnd.ms-fontobject"}, {".epub", "application/epub+zip"},
        {".gz", "application/gzip"}, {".gif", "image/gif"},
        {".htm", "text/html"}, {".html", "text/html"},
        {".ico", "image/vnd.microsoft.icon"}, {".ics", "text/calendar"},
        {".jar", "application/java-archive"}, {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"}, {".js","text/javascript"},
        {".json", "application/json"}, {".jsonld", "application/ld+json"},
        {".md", "text/markdown"},
        {".mid", "audio/x-midi"}, {".midi",	"audio/x-midi"},
        {".mjs", "text/javascript"}, {".mp3", "audio/mpeg"},
        {".mp4", "video/mp4"}, {".mpeg", "video/mpeg"},
        {".mpkg", "application/vnd.apple.installer+xml"}, {".odp", "application/vnd.oasis.opendocument.presentation"},
        {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {".odt", "application/vnd.oasis.opendocument.text"},
        {".oga", "audio/ogg"}, {".ogv", "video/ogg"},
        {".ogx", "application/ogg"}, {".opus", "audio/ogg"},
        {".otf", "font/otf"}, {".png", "image/png"},
        {".pdf", "application/pdf"}, {".php", "application/x-httpd-php"},
        {".ppt", "application/vnd.ms-powerpoint"}, {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {".rar", "application/vnd.rar"}, {".rtf", "application/rtf"},
        {".sh", "application/x-sh"}, {".svg", "image/svg+xml"}, 
        {".tar", "application/x-tar"}, {".tif", "image/tiff"},
        {".tiff", "image/tiff"}, {".ts", "video/mp2t"},
        {".ttf", "font/ttf"}, {".txt", "text/plain"},
        {".vsd", "application/vnd.visio"}, {".wav", "audio/wav"},
        {".weba", "audio/webm"}, {".webm", "video/webm"},
        {".webp", "image/webp"}, {".woff", "font/woff"},
        {".woff2", "font/woff2"}, {".xhtml", "application/xhtml+xml"},
        {".xls", "application/vnd.ms-excel"}, {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {".xml", "application/xml"}, {".xul", "application/vnd.mozilla.xul+xml"},
        {".zip", "application/zip"}, {".3gp", "video/3gpp"},
        {".3g2", "video/3gpp2"}, {".7z", "application/x-7z-compressed"}
    };
    return types.count(ext) ? types[ext] : "application/octet-stream";
}

// static std::string extractFilename(const std::string& path, int method)
// {
//     size_t start;
//     if (method)
//     {
//         size_t start = path.find("filename=\"");
//         if (start == std::string::npos)
//             return "";
//         start += 10;
//         size_t end = path.find("\"", start);
//         if (end == std::string::npos)
//             return "";
//         return path.substr(start, end - start);
//     }
//     else
//     {
//         start = path.find_last_of('/');
//         if (start == std::string::npos)
//             return path;
//         return path.substr(start + 1);
//     }
// }

// static std::string extractContent(const std::string& part)
// {
//     size_t start = part.find("\r\n\r\n");
//     size_t offset = 4;
//     if (start == std::string::npos)
//     {
//         start = part.find("\n\n");
//         offset = 2;
//     }
//     if (start == std::string::npos)
//         return "";
//     size_t conStart = start + offset;
//     size_t conEnd = part.find_last_not_of("\r\n") + 1;
//     if (conEnd <= conStart)
//         return "";
//     return part.substr(conStart, conEnd - conStart);
// }

// static std::vector<std::string> split(const std::string& s, const std::string& s2)
// {
//     std::vector<std::string> result;
//     size_t pos = 0;
//     while (true)
//     {
//         size_t start = s.find(s2, pos);
//         if (start == std::string::npos)
//             break;
//         start += s2.length();
//         while (start < s.size() && (s[start] == '-' || s[start] == '\r' || s[start] == '\n'))
//             start++;
//         size_t end = s.find(s2, start);
//         std::string part = (end == std::string::npos) ? s.substr(start) : s.substr(start, end - start);
//         while (!part.empty() && (part[0] == '\r' || part[0] == '\n'))
//             part.erase(0, 1);
//         while (!part.empty() && (part.back() == '\r' || part.back() == '\n'))
//             part.pop_back();
//         if (!part.empty())
//             result.push_back(part);
//         if (end == std::string::npos)
//             break;
//         pos = end;
//     }
//     return result;
// }

HTTPResponse generateSuccessResponse(std::string body, std::string type)
{
    HTTPResponse response(200, "OK");
    response.body = body;
    response.headers["Content-Type"] = type;//"text/html";
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return response;
}

static HTTPResponse generateIndexListing(std::string fullPath, std::string location)
{
    DIR* dir = opendir(fullPath.c_str());
    if (!dir)
        return HTTPResponse(500, "Failed to open directory");
    std::stringstream html;
    html << "<html><head><title>" << location << "</title></head><body>\n";
    html << "<h1 style=\"font-family:sans-serif\">" << location << "</h1><ul>\n";
    html << "<table cellpadding=\"5\" cellspacing=\"0\" style=\"text-align: left; font-family: sans-serif\">\n";
    html << "<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name =="." || name == "..")
            continue ;
        std::string ref = location;
        if (!ref.empty() && ref.back() != '/')
            ref += '/';
        ref += name;
        struct stat st;
        std::string fullEntry = fullPath + "/" + name;
        if (stat(fullEntry.c_str(), &st) == -1)
            continue;
        std::string displayedName = name;
        if (displayedName.length() > 15)
            displayedName = displayedName.substr(0, 12) + "...";
        if (entry->d_type == DT_DIR)
        {
            ref += "/";
            displayedName += "/";
        }
        char time[64];
        std::strftime(time, sizeof(time), "%Y-%m-%d %H:%M", std::localtime(&st.st_mtime));
        std::string size = (S_ISDIR(st.st_mode)) ? "-" : std::to_string(st.st_size) + " B";
        html << "<tr><td><a href=\"" << ref << "\">" << displayedName << "</a></td>"
             << "<td>" << time << "</td>"
             << "<td>" << size << "</td></tr>\n";
    }
    html << "</table></body></html>\n";
    closedir(dir);
    // HTTPResponse response(200, "OK");
    // response.body = html.str();
    // response.headers["Content-Type"] = "text/html";
    // response.headers["Content-Length"] = std::to_string(response.body.size());
    wslog.writeToLogFile(INFO, "GET Index listing successful", false);
    return generateSuccessResponse(html.str(), "text/html");
    // return response;
}

HTTPResponse RequestHandler::handleMultipart(Client& client)
{
    // std::cout << "Key: {" << client.request.location << "}" << std::endl;
    if (client.request.headers.count("Content-Type") == 0)
        return HTTPResponse(400, "Missing Content-Type");
    auto its = client.request.headers.find("Content-Type");
    std::string ct = its->second;
    if (its == client.request.headers.end())
        return HTTPResponse(400, "Invalid headers");
    std::string boundary;
    std::string::size_type pos = ct.find("boundary=");
    if (pos == std::string::npos)
        return HTTPResponse (400, "No boundary");
    boundary = ct.substr(pos + 9);
    if (!boundary.empty() && boundary[0] == '"')
        boundary = boundary.substr(1, boundary.find('"', 1) - 1);
    std::string bound_mark = "--" + boundary;
    std::vector<std::string> parts = split(client.request.body, bound_mark);
    std::string lastPath;
    for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it)
    {
        std::string& part = *it;
        if (part.empty() || part == "--\r\n" || part == "--")
            continue;
        std::string disposition = part.substr(0, part.find("\r\n"));
        if (disposition.find("filename=\"") == std::string::npos)
            continue; 
        std::string file = extractFilename(part, 1);
        // wslog.writeToLogFile(INFO, "File: " + file, false);
        if (file.empty())
            continue;
        std::string content = extractContent(part);
        std::string folder = client.serverInfo.routes[client.request.location].abspath;
        // wslog.writeToLogFile(INFO, "Folder: " + folder, false);
        std::string path = folder;
        if (path.back() != '/')
            path += "/";
        path += file;
        lastPath = path;
        //wslog.writeToLogFile(INFO, "Path: " + lastPath, false);
        std::ofstream out(path.c_str(), std::ios::binary);
        if (!out.is_open())
        {
            wslog.writeToLogFile(ERROR, "500 Failed to open file for writing", false);
            return HTTPResponse(500, "Failed to open file for writing");
        }
        out.write(content.c_str(), content.size());
        out.close();
    }
    if (lastPath.empty() || access(lastPath.c_str(), R_OK) != 0)
        return HTTPResponse(400, "File not uploaded");
    std::string ext = getFileExtension(client.request.path);
    wslog.writeToLogFile(INFO, "POST (multi) File(s) uploaded successfully", false);
    return generateSuccessResponse("File(s) uploaded successfully\n", getMimeType(ext));
}

HTTPResponse RequestHandler::handlePOST(Client& client, std::string fullPath)
{
    if (client.request.headers.count("Content-Type") == 0)
        return HTTPResponse(400, "Missing Content-Type");
    // std::cout << "Content type: " << req.headers["Content-Type"] << std::endl;
    if (client.request.headers["Content-Type"].find("multipart/form-data") != std::string::npos)
        return handleMultipart(client);
    // std::cout << "Key: " << key << std::endl;
    // std::cout << "Path: " << path << std::endl;
    std::ofstream out(fullPath.c_str(), std::ios::binary);
    if (!out.is_open())
    {
        wslog.writeToLogFile(ERROR, "500 Failed to open file for writing", false);
        return HTTPResponse(500, "Failed to open file for writing");
    }
    out.write(client.request.body.c_str(), client.request.body.size());
    out.close();
    if (access(fullPath.c_str(), R_OK) != 0)
    {
        wslog.writeToLogFile(ERROR, "400 File not uploaded", false);
        return HTTPResponse(400, "File not uploaded");
    }
    if (client.request.file.empty())
    {
        wslog.writeToLogFile(ERROR, "400 Bad request", false);
        return HTTPResponse(400, "Bad request");
    }
    // HTTPResponse res(200, "OK");
    // res.body = "File(s) uploaded successfully\n";
    std::string ext = getFileExtension(client.request.path);
    // res.headers["Content-Type"] = getMimeType(ext);
    // res.headers["Content-Length"] = std::to_string(res.body.size());
    wslog.writeToLogFile(INFO, "POST File(s) uploaded successfully", false);
    return generateSuccessResponse("File(s) uploaded successfully\n", getMimeType(ext));
    // return res;
}

HTTPResponse RequestHandler::handleGET(Client& client, std::string fullPath)
{
    // wslog.writeToLogFile(INFO, "GET Path :" + fullPath, DEBUG_LOGS);
    // if (fullPath.find("..") != std::string::npos)
    // {
    //     wslog.writeToLogFile(ERROR, "403 Forbidden", false);
    //     return HTTPResponse(403, "Forbidden");
    // }
    struct stat s;
    if (stat(fullPath.c_str(), &s) != 0 || access(fullPath.c_str(), R_OK) != 0)
    {
        wslog.writeToLogFile(ERROR, "404 Not Found", false);
        return HTTPResponse(404, "Not Found");
    }
    bool isDir = S_ISDIR(s.st_mode);
    if (isDir && !client.serverInfo.routes[client.request.location].index_file.empty())
    {
        // wslog.writeToLogFile(DEBUG, "We are here", DEBUG_LOGS);
        fullPath = joinPaths(fullPath, client.serverInfo.routes[client.request.location].index_file);
        std::ifstream file(fullPath.c_str(), std::ios::binary);
        // wslog.writeToLogFile(DEBUG, "fullpath after file is open " + fullPath, DEBUG_LOGS);
        if (!file.is_open())
        {
            wslog.writeToLogFile(ERROR, "404, Not Found", false);
            return HTTPResponse(404, "Not Found");
        }
        std::ostringstream content;
        content << file.rdbuf();
        file.close();
        // wslog.writeToLogFile(DEBUG, "Content: " + content.str(), DEBUG_LOGS);
        std::string ext = getFileExtension(fullPath);
        wslog.writeToLogFile(INFO, "GET File(s) downloaded successfully", false);
        return generateSuccessResponse(content.str(), getMimeType(ext));
        // HTTPResponse response(200, "OK");
        // response.body = content.str();
        // response.headers["Content-Type"] = getMimeType(ext);
        // response.headers["Content-Length"] = std::to_string(response.body.size());
        // return response;
    }
    if (isDir && !client.serverInfo.routes[client.request.location].autoindex && client.serverInfo.routes[client.request.location].index_file.empty())
        return generateIndexListing(fullPath, client.request.location);
    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        wslog.writeToLogFile(ERROR, "500 Internal Server Error", false);
        return HTTPResponse(500, "Internal Server Error");
    }
    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    // wslog.writeToLogFile(INFO, "Content: " + content.str(), DEBUG_LOGS);
    std::string ext = getFileExtension(fullPath);
    wslog.writeToLogFile(INFO, "GET File(s) downloaded successfully", false);
    return generateSuccessResponse(content.str(), getMimeType(ext));
    // HTTPResponse response(200, "OK");
    // response.body = content.str();
    // response.headers["Content-Type"] = getMimeType(ext);
    // response.headers["Content-Length"] = std::to_string(response.body.size());
    // return response;
}

HTTPResponse RequestHandler::handleDELETE(std::string fullPath)
{
    // I think this /uploads/ can not be hardcoded the user can design the directory structure how he wants
    /// we need to think something for this. I think the allowedMethods checks is just enough.
    if (fullPath.find("..") != std::string::npos)
        return HTTPResponse(403, "Forbidden");
    if (access(fullPath.c_str(), F_OK) != 0)
        return HTTPResponse(404, "Not Found");
    if (remove(fullPath.c_str()) != 0)
        return HTTPResponse(500, "Delete Failed");
    // HTTPResponse res(200, "OK");
    // res.body = "File deleted successfully\n";
    // res.headers["Content-Type"] = "text/plain";
    // res.headers["Content-Length"] = std::to_string(res.body.size());
    wslog.writeToLogFile(INFO, "DELETE File deleted successfully", false);
    return generateSuccessResponse("File deleted successfully\n", "text/plain");
    // return res;
}


bool RequestHandler::isAllowedMethod(std::string method, Route route)
{
    for (size_t i = 0; i < route.accepted_methods.size(); i++)
    {
        if (method == route.accepted_methods[i])
            return true;
    }
    return false;
}

HTTPResponse RequestHandler::redirectResponse(std::string fullPath)
{
    fullPath += "/";
    HTTPResponse res(301, "Moved Permanently");
    res.headers["Location"] = fullPath;
    return res;
}

HTTPResponse RequestHandler::handleRequest(Client& client)
{
    // printRequest(client.request);
    for (size_t i = 0; i < client.request.file.size(); i++)
    {
        if (isspace(client.request.file[i]))
            return HTTPResponse(403, "Whitespace in filename");
    }
    std::string fullPath = "." + joinPaths(client.serverInfo.routes[client.request.location].abspath, client.request.file);
    wslog.writeToLogFile(DEBUG, "location is " + client.request.location, DEBUG_LOGS);
    wslog.writeToLogFile(DEBUG, "Request handler fullpath is " + fullPath, DEBUG_LOGS);
    // if (client.serverInfo.routes.find(client.request.location) == client.serverInfo.routes.end())
    //     return HTTPResponse(404, "Invalid file name");
    if (fullPath.find("..") != std::string::npos)
    {
        wslog.writeToLogFile(ERROR, "403 Forbidden", false);
        return HTTPResponse(403, "Forbidden");
    }
    bool validFile = false;
    try
    {
        validFile = std::filesystem::exists(fullPath);
    }
    catch(const std::exception& e)
    {
        wslog.writeToLogFile(ERROR, "Invalid file name", DEBUG_LOGS);
        return HTTPResponse(404, "Invalid file name");
    }
    if (!validFile)
        return HTTPResponse(404, "Invalid file");
    /// what if there is a directory and file with the same name...
    if (fullPath != "." && std::filesystem::is_regular_file(fullPath) == false && std::filesystem::is_directory(fullPath) && fullPath.back() != '/')
        return redirectResponse(client.request.file);
    if (!isAllowedMethod(client.request.method, client.serverInfo.routes[client.request.location]))
        return HTTPResponse(405, "Method not allowed");
    switch (client.request.eMethod)
    {
        case GET:
            return handleGET(client, fullPath);
        case POST:
            return handlePOST(client, fullPath);
        case DELETE:
            return handleDELETE(fullPath);
        default:
            return HTTPResponse(501, "Not Implemented");
    }
}