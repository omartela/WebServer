#include "CGIhandler.hpp"
#include "utils.hpp"
#include "utils.hpp"
#include <limits.h>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);

CGIHandler::CGIHandler() 
{
	writeCGIPipe[1] = -1;
	writeCGIPipe[0] = -1;
}

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
		//wslog.writeToLogFile(DEBUG, "CGIHandler::setEnvValues envArray[" + std::to_string(i) + "] = " + envVariables.at(i), true);
		//wslog.writeToLogFile(DEBUG, "CGIHandler::setEnvValues envArray[" + std::to_string(i) + "] = " + envVariables.at(i), true);
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
	// if (signum != 0)
    //     return ;
	//std::cout << "signum in collectCGIOutput = " << signum << std::endl;
	// if (signum != 0)
    //     return ;
	//std::cout << "signum in collectCGIOutput = " << signum << std::endl;
    char buffer[65536];
    int n;
    int n;
    //output.clear();

    // while ((n = read(readFd, buffer, sizeof(buffer)) > 0)
    //     output.append(buffer, n);

    n = read(childReadPipeFd, buffer, sizeof(buffer));
    if (n > 0)
        output.append(buffer, n);
    wslog.writeToLogFile(INFO, "Collected " + std::to_string(n) + " bytes from the child process", true);
	wslog.writeToLogFile(INFO, "Size of output = " + std::to_string(output.length()), true);
	wslog.writeToLogFile(INFO, "Size of output = " + std::to_string(output.length()), true);

    //close(readFd);
}

void CGIHandler::writeBodyToChild(HTTPRequest& request)
{
    // write(writeCGIPipe[1], client.request.body.c_str(), client.request.body.size());
    // close(writeCGIPipe[1]);
	// if (signum != 0)
    //     return ;
	//std::cout << "signum in writeBodyToChild = " << signum << std::endl;
    int written = write(writeCGIPipe[1], request.body.c_str(), request.body.size());
	// if (signum != 0)
    //     return ;
	//std::cout << "signum in writeBodyToChild = " << signum << std::endl;
    int written = write(writeCGIPipe[1], request.body.c_str(), request.body.size());
    if (written > 0) 
        request.body = request.body.substr(written);
    wslog.writeToLogFile(INFO, "Written to child pipe: " + std::to_string(written), true);
    if (request.body.empty() == true)
	{
        close(writeCGIPipe[1]);
		writeCGIPipe[1] = -1;
	}
		writeCGIPipe[1] = -1;
	}
}

int CGIHandler::executeCGI(HTTPRequest& request, ServerConfig server)
{
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI called", true);
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI fullPath is: " + fullPath, true);
	if (request.FileUsed)
	{
		tempFileName = "/tmp/tempCGIouput_" + std::to_string(std::time(NULL)); 
		readCGIPipe[1] =  open(tempFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
		FileOpen = true;
		request.FileFd = open(request.tempFileName.c_str(), O_RDONLY, 0644);
		if (request.FileFd == -1)
		{
			/// error
			return -1;
		}
		tempFileName = "/tmp/tempCGIouput_" + std::to_string(std::time(NULL)); 
		readCGIPipe[1] =  open(tempFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
		FileOpen = true;
		request.FileFd = open(request.tempFileName.c_str(), O_RDONLY, 0644);
		if (request.FileFd == -1)
		{
			/// error
			return -1;
		}
		writeCGIPipe[0] = request.FileFd;
	}
    if (access(fullPath.c_str(), X_OK) != 0)
    {
        wslog.writeToLogFile(ERROR, "CGIHandler::executeCGI access to cgi script forbidden: " + fullPath, true);
        return -403;
        return -403;
    }
    if (!request.FileUsed && (pipe(writeCGIPipe) == -1 || pipe(readCGIPipe) == -1))
        return -500;
    wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI pipes created", true);
    childPid = fork();
    if (childPid == -1)
        return -500;
        return -500;
    if (childPid == 0)
    {
        dup2(writeCGIPipe[0], STDIN_FILENO);
        dup2(readCGIPipe[1], STDOUT_FILENO);
		if (!request.FileUsed)
		{
			close(writeCGIPipe[1]);
			writeCGIPipe[1] = -1;
			writeCGIPipe[1] = -1;
			close(readCGIPipe[0]);
			readCGIPipe[0] = -1;
			readCGIPipe[0] = -1;
		}
        execve(server.routes[request.location].cgiexecutable.c_str(), exceveArgs, envArray);
        std::cout << "I WILL NOT GET HERE IF CHILD SCRIPT WAS SUCCESSFUL\n";
        _exit(1);
    }
	if (!request.FileUsed)
	{
		wslog.writeToLogFile(ERROR, "Closing writeCGIPipe[0] FD = " + std::to_string(writeCGIPipe[0]), true);
		wslog.writeToLogFile(ERROR, "Closing readCGIPipe[1] FD = " + std::to_string(readCGIPipe[1]), true);
		wslog.writeToLogFile(ERROR, "Closing writeCGIPipe[0] FD = " + std::to_string(writeCGIPipe[0]), true);
		wslog.writeToLogFile(ERROR, "Closing readCGIPipe[1] FD = " + std::to_string(readCGIPipe[1]), true);
		close(writeCGIPipe[0]);
		writeCGIPipe[0] = -1;
		writeCGIPipe[0] = -1;
		close(readCGIPipe[1]);
		readCGIPipe[1] = -1;
		readCGIPipe[1] = -1;
	}
	if (!request.FileUsed)
	{
		int flags = fcntl(writeCGIPipe[1], F_GETFL); //save the previous flags if any
		fcntl(writeCGIPipe[1], F_SETFL, flags | O_NONBLOCK); //add non-blocking flag
		flags = fcntl(readCGIPipe[0], F_GETFL);
		fcntl(readCGIPipe[0], F_SETFL, flags | O_NONBLOCK);
	}
	if (!request.FileUsed)
	{
		int flags = fcntl(writeCGIPipe[1], F_GETFL); //save the previous flags if any
		fcntl(writeCGIPipe[1], F_SETFL, flags | O_NONBLOCK); //add non-blocking flag
		flags = fcntl(readCGIPipe[0], F_GETFL);
		fcntl(readCGIPipe[0], F_SETFL, flags | O_NONBLOCK);
	}
	return 0;
}

