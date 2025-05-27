#include "CGIhandler.hpp"
#include "Logger.hpp"
#include <limits.h>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);

CGIHandler::CGIHandler() {

}

// std::string  join_paths(const std::string& a, const std::string& b)
// {
// 	if (a.empty()) return b;
// 	if (b.empty()) return a;
// 	if (a[a.size() - 1] == '/' && b[0] =='/')
// 		return a + b.substr(1);
// 	if (a[a.size() -1] != '/' && b[0] != '/')
// 		return a + "/" + b;
// 	return a + b;
// }

void CGIHandler::setEnvValues(Client client)
{
	std::string server_name = client.serverInfo.server_names.empty() ? "localhost"
			: client.serverInfo.server_names.at(0);
	char absPath[PATH_MAX];
	std::string localPath = join_paths(client.serverInfo.routes.at(client.request.location).abspath, client.request.file);
	fullPath = "." + localPath;
	realpath(fullPath.c_str(), absPath);
	envVariables.clear();
	envVariables = {"REQUEST_METHOD=" + client.request.method,
					"SCRIPT_FILENAME=" + std::string(absPath),
					"SCRIPT_NAME=" + client.request.path,
					"QUERY_STRING=" + client.request.query,
					"PATH_INFO=" + client.request.pathInfo,
					"REDIRECT_STATUS=200",
					"SERVER_PROTOCOL=HTTP/1.1",
					"GATEWAY_INTERFACE=CGI/1.1",
					"REMOTE_ADDR=" + client.serverInfo.host,
					"SERVER_NAME=" + server_name,
					"SERVER_PORT=" + client.serverInfo.port};
	std::string conType =  client.request.headers.count("Content-Type") > 0 ? client.request.headers.at("Content-Type") : "text/plain";
	envVariables.push_back("CONTENT_TYPE=" + conType);
	std::string conLen = client.request.headers.count("Content-Length") > 0 ? client.request.headers.at("Content-Length") : "0";
	envVariables.push_back("CONTENT_LENGTH=" + conLen);
	for (size_t i = 0; i < envVariables.size(); i++)
		envArray[i] = (char *)envVariables.at(i).c_str();
	envArray[envVariables.size()] = NULL;
	exceveArgs[0] = (char *)client.serverInfo.routes.at(client.request.location).cgiexecutable.c_str();
	exceveArgs[1] = absPath;
	exceveArgs[2] = NULL;
}

HTTPResponse CGIHandler::generateCGIResponse()
{
	// std::cout << "GENERATING CGI RESPONSE\n";
	std::string::size_type end = output.find("\r\n\r\n");
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
	return res;
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
	// wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI called", true);
	// wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI fullPath is: " + fullPath, true);
	if (access(fullPath.c_str(), X_OK) != 0)
		return -1; // ERROR PAGE access forbidden
	if (pipe(writeCGIPipe) == -1 || pipe(readCGIPipe) == -1)
		return -1;
	// wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI pipes created", true);
	childPid = fork();
	if (childPid != 0)
		// wslog.writeToLogFile(DEBUG, "CGIHandler::executeCGI childPid is: " + std::to_string(childPid), true);
	if (childPid == -1)
		return -1;
	if (childPid == 0)
	{
		dup2(writeCGIPipe[0], STDIN_FILENO);
		dup2(readCGIPipe[1], STDOUT_FILENO);
		close(writeCGIPipe[1]);
		close(readCGIPipe[0]);
		execve(client.serverInfo.routes[client.request.location].cgiexecutable.c_str(), exceveArgs, envArray);
		// std::cout << "I WILL NOT GET HERE IF CHILD SCRIPT WAS SUCCESSFUL\n";
		_exit(1);
	}
	client.childPid = childPid;
	close(writeCGIPipe[0]);
	close(readCGIPipe[1]);
	return readCGIPipe[0];
}

// void registerCGI(Client& client, int epollFd, CGIHandler& cgi, int cgiOutFd)
// {
//     client.cgiPid = cgi.childPid;
//     client.cgiStdoutFd = cgiOutFd; //or register eventfd instead?
//     client.isCGI = true;

//     struct epoll_event ev;
//     ev.events = EPOLLIN;
//     ev.data.fd = cgiOutFd;
//     if (epoll_ctl(epollFd, EPOLL_CTL_ADD, cgiOutFd, &ev) < 0)
//         throw std::runtime_error("Failed to add CGI pipe to epoll");
// }

// HTTPResponse CGIHandler::executeCGI(Client& client)
// {
//     if (access(fullPath.c_str(), X_OK) != 0)
//         return  HTTPResponse(403, "Forbidden");
//     // int inPipe[2], outPipe[2];
//     pipe(writeCGIPipe);//inPipe
//     pipe(readCGIPipe);//outPipe
//     pid_t pid = fork();
//     if (pid == 0)
//     {
//         dup2(writeCGIPipe[0], STDIN_FILENO);
//         dup2(readCGIPipe[1], STDOUT_FILENO);
//         close(writeCGIPipe[1]);
//         close(readCGIPipe[0]);
//         execve(client.serverInfo.routes[client.request.location].cgiexecutable.c_str(), exceveArgs, envArray);
//         _exit(1);
//     }
//     close(writeCGIPipe[0]);
//     close(readCGIPipe[1]);
//     write(writeCGIPipe[1], client.request.body.c_str(), client.request.body.size());
//     close(readCGIPipe[1]);
//     char buffer[4096];
//     std::string output;
//     ssize_t n;
//     while ((n = read(readCGIPipe[0], buffer, sizeof(buffer))) > 0)
//         output.append(buffer, n);
//     close(readCGIPipe[0]);
//     int childPid;
//     childPid = waitpid(pid, NULL, WNOHANG);
//     std::string::size_type end = output.find("\r\n\r\n");
//     if (end == std::string::npos)
//     return HTTPResponse(500, "Invalid CGI output");
//     if (childPid == 0)
//     {
//         std::string headers = output.substr(0, end);
//         std::string body = output.substr(end + 4);
//         HTTPResponse res(200, "OK");
//         res.body = body;
//         std::istringstream header(headers);
//         std::string line;
//         while (std::getline(header, line))
//         {
//             if (line.back() == '\r')
//                 line.pop_back();
//             size_t colon = line.find(':');
//             if (colon != std::string::npos)
//                 res.headers[line.substr(0, colon)] = line.substr(colon + 2);
//         }
//         res.headers["Content-Length"] = std::to_string(res.body.size());
//         return res;
//     }
// }