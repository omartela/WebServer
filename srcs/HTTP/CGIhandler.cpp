#include "CGIhandler.hpp"
#include <limits.h>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);

CGIHandler::CGIHandler() { }

int CGIHandler::getWritePipe() { return writeCGIPipe[1]; }

int CGIHandler::getReadPipe() {return readCGIPipe[0]; }

int CGIHandler::getChildPid() { return childPid; }

void CGIHandler::setEnvValues(HTTPRequest& request, ServerConfig server)
{
	std::string server_name = server.server_names.empty() ? "localhost"
			: server.server_names.at(0);
	char absPath[PATH_MAX];
	std::string localPath = join_paths(server.routes.at(request.location).abspath, request.file);
	fullPath = "." + localPath;
	realpath(fullPath.c_str(), absPath);
	envVariables.clear();
	std::string PATH_INFO = request.pathInfo.empty() ? request.path : request.pathInfo;
	envVariables = {"REQUEST_METHOD=" + request.method,
					"SCRIPT_FILENAME=" + std::string(absPath),
					"SCRIPT_NAME=" + request.path,
					"QUERY_STRING=" + request.query,
					"PATH_INFO=" + PATH_INFO,
					"REDIRECT_STATUS=200",
					"SERVER_PROTOCOL=HTTP/1.1",
					"GATEWAY_INTERFACE=CGI/1.1",
					"REMOTE_ADDR=" + server.host,
					"SERVER_NAME=" + server_name,
					"SERVER_PORT=" + server.port};
	std::string conType =  request.headers.count("Content-Type") > 0 ? request.headers.at("Content-Type") : "text/plain";
	envVariables.push_back("CONTENT_TYPE=" + conType);
	std::string conLen = request.headers.count("Content-Length") > 0 ? request.headers.at("Content-Length") : "0";
	envVariables.push_back("CONTENT_LENGTH=" + conLen);
	for (size_t i = 0; i < envVariables.size(); i++)
	{
		envArray[i] = (char *)envVariables.at(i).c_str();
		wslog.writeToLogFile(DEBUG, "CGIHandler::setEnvValues envArray[" + std::to_string(i) + "] = " + envVariables.at(i), true);
	}
	envArray[envVariables.size()] = NULL;
	exceveArgs[0] = (char *)server.routes.at(request.location).cgiexecutable.c_str();
	exceveArgs[1] = absPath;
	exceveArgs[2] = NULL;
}

HTTPResponse CGIHandler::generateCGIResponse()
{
	// std::cout << "GENERATING CGI RESPONSE\n";
	std::string::size_type end = output.find("\r\n\r\n");
    wslog.writeToLogFile(DEBUG, output.substr(0, 100), true);
	if (end == std::string::npos)
		return HTTPResponse(500, "Invalid CGI output");
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
    //output.clear //?
	return res;
}

void CGIHandler::collectCGIOutput(int childReadPipeFd)
{
    char buffer[65536];
    ssize_t n;
    //output.clear();

    // while ((n = read(readFd, buffer, sizeof(buffer)) > 0)
    //     output.append(buffer, n);

    n = read(childReadPipeFd, buffer, sizeof(buffer));
    if (n > 0)
        output.append(buffer, n);
    wslog.writeToLogFile(INFO, "Collected " + std::to_string(n) + " bytes from the child process", true);

    //close(readFd);
}

void CGIHandler::writeBodyToChild(HTTPRequest& request)
{
    // write(writeCGIPipe[1], client.request.body.c_str(), client.request.body.size());
    // close(writeCGIPipe[1]);
    size_t written = write(writeCGIPipe[1], request.body.c_str(), request.body.size());
    if (written > 0) 
        request.body = request.body.substr(written);
    wslog.writeToLogFile(INFO, "Written to child pipe: " + std::to_string(written), true);
    if (request.body.empty() == true)
        close(writeCGIPipe[1]);
}

void CGIHandler::executeCGI(HTTPRequest& request, ServerConfig server)
{
	/// kato flagi FileIsUsed jos flagi paalla ala pipee
	/// Duppaa se tiedoston fd STDIN_FILENO
	/// Avaa uus tiedosto cgi vastauksen kirjoittamista varten
	/// Duppaa STDOUT_FILENO uuden tiedoston fd:hen.

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
        execve(server.routes[request.location].cgiexecutable.c_str(), exceveArgs, envArray);
        std::cout << "I WILL NOT GET HERE IF CHILD SCRIPT WAS SUCCESSFUL\n";
        _exit(1);
    }
	// Mieti mitka fd suljetaan kun on tiedosto kaytossa...

    //client.childPid = childPid;
	close(writeCGIPipe[0]);
	close(readCGIPipe[1]);

    int flags = fcntl(writeCGIPipe[1], F_GETFL); //save the previous flags if any
    fcntl(writeCGIPipe[1], F_SETFL, flags | O_NONBLOCK); //add non-blocking flag
    flags = fcntl(readCGIPipe[0], F_GETFL);
    fcntl(readCGIPipe[0], F_SETFL, flags | O_NONBLOCK);

    // client.childWritePipeFd = writeCGIPipe[1];
    // client.childReadPipeFd = readCGIPipe[0];

	return ;
}
