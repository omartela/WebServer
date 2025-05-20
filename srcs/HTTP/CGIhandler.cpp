#include "CGIhandler.hpp"

CGIHandler::CGIHandler() {

}

void CGIHandler::setEnvValues(Client client)
{
    fullPath = "." + client.serverInfo.routes[client.request.location].abspath + client.request.file;
    envVariables = {"CONTENT_LENGTH =", "CONTENT_TYPE=", "QUERY_STRING=" + client.request.query, "PATH_INFO=" + client.request.pathInfo,
                    "REQUEST_METHOD=" + client.request.method, "SCRIPT_FILENAME=" + fullPath, "SCRIPT_NAME=" + client.request.path, "REDIRECT_STATUS=200",
                    "SERVER_PROTOCOL=HTTP/1.1", "GATEWAY_INTERFACE=CGI/1.1", "REMOTE_ADDR=" + client.serverInfo.host,
                    "SERVER_NAME=" + client.serverInfo.server_names.at(0), "SERVER_PORT=" + client.serverInfo.port, NULL};
    if (client.request.headers.find("Content-Length") != client.request.headers.end())
        envVariables.at(0) += client.request.headers.at("Content-Length");
    if (client.request.headers.find("Content-Type") != client.request.headers.end())
        envVariables.at(0) += client.request.headers.at("Content-Type");
    for (int i = 0; i < 14; i++)
        envArray[i] = (char *) envVariables.at(i).c_str();
    if (client.request.file.size() >= 3 && client.request.file.compare(client.request.file .size() - 3, 3, "php") == 0)
        exceveArgs[0] = (char *) client.serverInfo.routes.at(client.request.location).cgipathphp.c_str();
    else
        exceveArgs[0] = (char *) client.serverInfo.routes.at(client.request.location).cgipathpython.c_str();
    exceveArgs[1] = (char *) fullPath.c_str();
    exceveArgs[2] = NULL;
}

HTTPResponse CGIHandler::executeCGI(Client& client)
{
    if (access(fullPath.c_str(), X_OK) != 0)
        return  HTTPResponse(403, "Forbidden");
    // int inPipe[2], outPipe[2];
    pipe(writeCGIPipe);//inPipe
    pipe(readCGIPipe);//outPipe
    pid_t pid = fork();
    if (pid == 0)
    {
        dup2(writeCGIPipe[0], STDIN_FILENO);
        dup2(readCGIPipe[1], STDOUT_FILENO);
        close(writeCGIPipe[1]);
        close(readCGIPipe[0]);
        execve(client.serverInfo.routes[client.request.location].cgipathpython.c_str(), exceveArgs, envArray);
        _exit(1);
    }
    close(writeCGIPipe[0]);
    close(readCGIPipe[1]);
    write(writeCGIPipe[1], client.request.body.c_str(), client.request.body.size());
    close(readCGIPipe[1]);
    char buffer[4096];
    std::string output;
    ssize_t n;
    while ((n = read(readCGIPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, n);
    close(readCGIPipe[0]);
    int childPid;
    childPid = waitpid(pid, NULL, WNOHANG);
    std::string::size_type end = output.find("\r\n\r\n");
    if (end == std::string::npos)
    return HTTPResponse(500, "Invalid CGI output");
    if (childPid == 0)
    {
        std::string headers = output.substr(0, end);
        std::string body = output.substr(end + 4);
        HTTPResponse res(200, "OK");
        res.body = body;
        std::istringstream header(headers);
        std::string line;
        while (std::getline(header, line))
        {
            if (line.back() == '\r')
                line.pop_back();
            size_t colon = line.find(':');
            if (colon != std::string::npos)
                res.headers[line.substr(0, colon)] = line.substr(colon + 2);
        }
        res.headers["Content-Length"] = std::to_string(res.body.size());
        return res;
    }
}