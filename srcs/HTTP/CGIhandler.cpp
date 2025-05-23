#include "CGIhandler.hpp"

CGIHandler::CGIHandler() {

}

void CGIHandler::setEnvValues(Client client)
{
    fullPath = "." + client.serverInfo.routes.at(client.request.location).abspath + client.request.file;
    envVariables = {"CONTENT_LENGTH =", "CONTENT_TYPE=", "QUERY_STRING=" + client.request.query, "PATH_INFO=" + client.request.pathInfo,
                    "REQUEST_METHOD=" + client.request.method, "SCRIPT_FILENAME=" + fullPath, "SCRIPT_NAME=" + client.request.path, "REDIRECT_STATUS=200",
                    "SERVER_PROTOCOL=HTTP/1.1", "GATEWAY_INTERFACE=CGI/1.1", "REMOTE_ADDR=" + client.serverInfo.host,
                    "SERVER_NAME=" + client.serverInfo.server_names.at(0), "SERVER_PORT=" + std::to_string(client.serverInfo.port)};
    if (client.request.headers.find("Content-Length") != client.request.headers.end())
        envVariables.at(0) += client.request.headers.at("Content-Length");
    if (client.request.headers.find("Content-Type") != client.request.headers.end())
        envVariables.at(0) += client.request.headers.at("Content-Type");
    for (size_t i = 0; i < envVariables.size(); i++)
        envArray[i] = (char *) envVariables.at(i).c_str();
    envArray[envVariables.size()] = NULL;
    exceveArgs[0] = (char *) client.serverInfo.routes.at(client.request.location).cgiexecutable.c_str();
    exceveArgs[1] = (char *) fullPath.c_str();
    exceveArgs[2] = NULL;
}

HTTPResponse CGIHandler::generateCGIResponse()
{
    std::cout << "GENERATING CGI RESPONSE\n";
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

void CGIHandler::collectCGIOutput(int readFd)
{
    char buffer[4096];
    ssize_t n;
    output.clear();

    while ((n = read(readFd, buffer, sizeof(buffer))) > 0)
        output.append(buffer, n);

    close(readFd);
}

int CGIHandler::executeCGI(Client& client)
{
    std::cout << fullPath << std::endl;
    if (access(fullPath.c_str(), X_OK) != 0)
        return -1; //ERROR PAGE access forbidden
    if (pipe(writeCGIPipe) == -1 || pipe(readCGIPipe) == -1)
        return -1;
    childPid = fork();
    if (childPid == -1)
        return -1;
    if (childPid == 0)
    {
        dup2(writeCGIPipe[0], STDIN_FILENO);
        dup2(readCGIPipe[1], STDOUT_FILENO);
        close(writeCGIPipe[1]);
        close(readCGIPipe[0]);
        execve(client.serverInfo.routes[client.request.location].cgiexecutable.c_str(), exceveArgs, envArray);
        _exit(1);
    }
    client.childPid = childPid;
	close(writeCGIPipe[0]);
	close(readCGIPipe[1]);
	if (!client.request.body.empty())
		write(writeCGIPipe[1], client.request.body.c_str(), client.request.body.size());
    close(writeCGIPipe[1]);
	return readCGIPipe[0];
}