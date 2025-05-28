#include "CGIhandler.hpp"
#include "Logger.hpp"

CGIHandler::CGIHandler() { }

void CGIHandler::setEnvValues(Client client)
{
    std::string server_name;
    if (!client.serverInfo.server_names.empty())
        server_name =  client.serverInfo.server_names.at(0);
    fullPath = "." + join_paths(client.serverInfo.routes.at(client.request.location).abspath, client.request.file);
    envVariables = {"CONTENT_LENGTH =", "CONTENT_TYPE=", "QUERY_STRING=" + client.request.query, "PATH_INFO=" + client.request.pathInfo,
                    "REQUEST_METHOD=" + client.request.method, "SCRIPT_FILENAME=" + fullPath, "SCRIPT_NAME=" + client.request.path, "REDIRECT_STATUS=200",
                    "SERVER_PROTOCOL=HTTP/1.1", "GATEWAY_INTERFACE=CGI/1.1", "REMOTE_ADDR=" + client.serverInfo.host,
                    "SERVER_NAME=" + server_name, "SERVER_PORT=" + client.serverInfo.port};
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

// void CGIHandler::setEnvValues(Client client)
// {
// 	std::string server_name = client.serverInfo.server_names.empty() ? "localhost"
// 			: client.serverInfo.server_names.at(0);
// 	char absPath[PATH_MAX];
// 	std::string localPath = join_paths(client.serverInfo.routes.at(client.request.location).abspath, client.request.file);
// 	fullPath = "." + localPath;
// 	realpath(fullPath.c_str(), absPath);
// 	envVariables.clear();
// 	envVariables = {"REQUEST_METHOD=" + client.request.method,
// 					"SCRIPT_FILENAME=" + std::string(absPath),
// 					"SCRIPT_NAME=" + client.request.path,
// 					"QUERY_STRING=" + client.request.query,
// 					"PATH_INFO=" + client.request.pathInfo,
// 					"REDIRECT_STATUS=200",
// 					"SERVER_PROTOCOL=HTTP/1.1",
// 					"GATEWAY_INTERFACE=CGI/1.1",
// 					"REMOTE_ADDR=" + client.serverInfo.host,
// 					"SERVER_NAME=" + server_name,
// 					"SERVER_PORT=" + client.serverInfo.port};
// 	std::string conType =  client.request.headers.count("Content-Type") > 0 ? client.request.headers.at("Content-Type") : "text/plain";
// 	envVariables.push_back("CONTENT_TYPE=" + conType);
// 	std::string conLen = client.request.headers.count("Content-Length") > 0 ? client.request.headers.at("Content-Length") : "0";
// 	envVariables.push_back("CONTENT_LENGTH=" + conLen);
// 	for (size_t i = 0; i < envVariables.size(); i++)
// 		envArray[i] = (char *)envVariables.at(i).c_str();
// 	envArray[envVariables.size()] = NULL;
// 	exceveArgs[0] = (char *)client.serverInfo.routes.at(client.request.location).cgiexecutable.c_str();
// 	exceveArgs[1] = absPath;
// 	exceveArgs[2] = NULL;
// }

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
    output.clear();
    return res;
}

void CGIHandler::setOutput(std::string newOutput) 
{
    this->output = newOutput;
};

void CGIHandler::collectCGIOutput(Client& client)
{
    char buffer[4096];
    ssize_t n;
    //output.clear();

    // while ((n = read(readFd, buffer, sizeof(buffer)) > 0)
    //     output.append(buffer, n);

    n = read(client.childReadPipeFd, buffer, sizeof(buffer));
    if (n > 0)
        client.CGIOutput.append(buffer, n);
    wslog.writeToLogFile(INFO, "Collected " + std::to_string(n) + " bytes from the child process", true);

    //close(readFd);
}

void CGIHandler::writeBodyToChild(Client& client)
{
    // write(writeCGIPipe[1], client.request.body.c_str(), client.request.body.size());
    // close(writeCGIPipe[1]);
    size_t written = write(client.childWritePipeFd, client.request.body.c_str(), client.request.body.size());
    if (written > 0) 
        client.request.body = client.request.body.substr(written);
    wslog.writeToLogFile(INFO, "Written to child pipe: " + std::to_string(written), true);
    if (client.request.body.empty() == true)
        close(client.childWritePipeFd);
}

void CGIHandler::executeCGI(Client& client)
{
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI called", true);
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI fullPath is: " + fullPath, true);
    if (access(fullPath.c_str(), X_OK) != 0)
    {
        wslog.writeToLogFile(ERROR, "CGIHandler::executeCGI access to cgi script forbidden: " + fullPath, true);
        return ; //generate ERROR PAGE access forbidden
    }
    if (pipe(writeCGIPipe) == -1 || pipe(readCGIPipe) == -1)
        throw std::runtime_error("CGIHandler::executeCGI pipe creation failed");
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI pipes created", true);
    childPid = fork();
    if (childPid != 0)
        wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI childPid is: " + std::to_string(childPid), true);
    if (childPid == -1)
        throw std::runtime_error("CGIHandler::executeCGI fork failed");
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

    int flags = fcntl(writeCGIPipe[1], F_GETFL); //save the previous flags if any
    fcntl(writeCGIPipe[1], F_SETFL, flags | O_NONBLOCK); //add non-blocking flag
    flags = fcntl(readCGIPipe[0], F_GETFL);
    fcntl(readCGIPipe[0], F_SETFL, flags | O_NONBLOCK);

    client.childWritePipeFd = writeCGIPipe[1];
    client.childReadPipeFd = readCGIPipe[0];

	return ;
}