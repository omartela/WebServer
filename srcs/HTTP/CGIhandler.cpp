#include "CGIhandler.hpp"
#include "Logger.hpp"



CGIHandler::CGIHandler() {

}

void CGIHandler::setEnvValues(Client client)
{
    std::string server_name;
    if (!client.serverInfo.server_names.empty())
        server_name =  client.serverInfo.server_names.at(0);
    fullPath = "." + join_paths(client.serverInfo.routes.at(client.request.location).abspath, client.request.file);
    // If pathInfo is empty, set it to the script name (or another default)
    std::string path_info = client.request.pathInfo.empty() ? "aaa" : client.request.pathInfo;

    wslog.writeToLogFile(DEBUG, "CGIHandler::setEnvValues pathinfo is " + path_info, true);
    envVariables = {"PATH_INFO=" + path_info,
                    "REQUEST_METHOD=" + client.request.method,
                    "SERVER_PROTOCOL=HTTP/1.1"};
    if (client.request.headers.find("Content-Length") != client.request.headers.end())
        envVariables.at(0) += client.request.headers.at("Content-Length");
    if (client.request.headers.find("Content-Type") != client.request.headers.end())
        envVariables.at(1) += client.request.headers.at("Content-Type");
    for (size_t i = 0; i < envVariables.size(); i++)
    {
        wslog.writeToLogFile(DEBUG, "CGIHandler::setEnvValues envVariables[" + std::to_string(i) + "] = " + envVariables.at(i), true);
        envArray[i] = (char *) envVariables.at(i).c_str();
    }
    envArray[envVariables.size()] = NULL;
    exceveArgs[0] = (char *) client.serverInfo.routes.at(client.request.location).cgiexecutable.c_str();
    exceveArgs[1] = (char *) fullPath.c_str();
    exceveArgs[2] = NULL;
}

HTTPResponse CGIHandler::generateCGIResponse()
{
    //std::cout << "GENERATING CGI RESPONSE\n";
    std::string::size_type end = output.find("\r\n\r\n");
    if (end == std::string::npos)
    {
        return HTTPResponse(500, "Invalid CGI output");
    }
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
    /*
    2 options, as i understand it
    1. register one end of the pipe with epoll_ctl. we will get epollins everytime there is something to read and must read it to a buffer or somewhere
    2. we register eventfd that we trigger once child is ready, just before returning, but we must reserve an fd for this
    */

    //here alarm epoll that cgi response is ready and child is exiting
}

bool CGIHandler::isFdReadable(int fd) 
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Non-blocking

    int ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (ret > 0 && FD_ISSET(fd, &readfds)) {
        // Data is available to read
        return true;
    }
    return false; // No data available
}

bool CGIHandler::isFdWritable(int fd) 
{
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Non-blocking

    int ret = select(fd + 1, NULL, &writefds, NULL, &timeout);
    if (ret > 0 && FD_ISSET(fd, &writefds)) {
        // FD is writable
        return true;
    }
    return false; // Not writable
}

void CGIHandler::collectCGIOutput(int readFd)
{
    char buffer[4096];
    ssize_t n;

    while ((n = read(readFd, buffer, sizeof(buffer))) > 0)
        output.append(buffer, n);
}

int CGIHandler::executeCGI(Client& client)
{
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI called", true);
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI fullPath is: " + fullPath, true);
    if (access(fullPath.c_str(), X_OK) != 0)
        return -1; //ERROR PAGE access forbidden
    if (pipe(writeCGIPipe) == -1 || pipe(readCGIPipe) == -1)
        return -1;
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI pipes created", true);
    childPid = fork();
    if (childPid != 0)
        wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI childPid is: " + std::to_string(childPid), true);
    if (childPid == -1)
        return -1;
    if (childPid == 0)
    {
        dup2(writeCGIPipe[0], STDIN_FILENO);
        dup2(readCGIPipe[1], STDOUT_FILENO);
        close(writeCGIPipe[1]);
        close(readCGIPipe[0]);
        execve(client.serverInfo.routes[client.request.location].cgiexecutable.c_str(), exceveArgs, envArray);
        std::cout << "I WILL NOT GET HERE IF CHILD SCRIPT WAS SUCCESSFUL\n";
        _exit(1);
    }
    client.childPid = childPid;
	close(writeCGIPipe[0]);
	close(readCGIPipe[1]);
	return readCGIPipe[0];
}