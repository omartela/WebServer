#include "Client.hpp"

Client::Client()
{
    this->fd = -1;
    this->state = IDLE;
    this->timeConnected = 0;
    this->bytesRead = 0;
    this->bytesWritten = 0;
    // this->request.contentLen = 0;
}

Client::~Client() {
}

Client::Client(const Client& copy)
{
    *this = copy;
}

Client& Client::operator=(const Client& copy)
{
    if (this != & copy)
    {
        this->fd = copy.fd;
        this->state = copy.state;
        this->readBuffer = copy.readBuffer;
        this->rawRequest = copy.rawRequest;
        this->writeBuffer = copy.writeBuffer;
        this->bytesRead = copy.bytesRead;
        this->bytesWritten = copy.bytesWritten;
        this->serverInfo = copy.serverInfo;
        this->request = copy.request;
    }
    return *this;
}

void Client::reset()
{
    this->state = IDLE;
    this->readBuffer.clear();
    this->rawRequest.clear();
    this->writeBuffer.clear();
    this->bytesRead = 0;
    this->bytesWritten = 0;
    this->request = HTTPRequest();
}

// void Client::resetRequest()
// {
//     this->request.method.clear();
//     this->request.path.clear();
//     this->request.version.clear();
//     this->request.body.clear();
//     this->request.headers.clear();
//     // this->request.contentLen = 0;
// }

// void Client::requestParser()
// {
//     std::istringstream stream(headerString);
//     std::string line;
//     if (std::getline(stream, line).fail())
//         return ;
//     if (line.back() == '\r') //remove trailing \r
//         line.pop_back();
//     std::istringstream request(line);
//     request >> this->request.method >> this->request.path >> this->request.version; //extracts the values separated by spaces to variables
//     while (std::getline(stream, line))
//     {
//         if (line.back() == '\r')
//             line.pop_back();
//         if (line.empty())
//             break ;
//         size_t colon = line.find(':');
//         if (colon != std::string::npos)
//         {
//             std::string key = line.substr(0, colon);
//             std::string value = line.substr(colon + 1);
//             this->removeWhitespaces(key, value);
//             this->request.headers[key] = value;
//         }
//     }
//     this->validateHeader();
//     this->request.eMethod = getMethodEnum();
// }

void Client::removeWhitespaces(std::string& key, std::string& value)
{
    if (key.empty() || value.empty())
        throw std::runtime_error("400 bad request"); //change to an actual response

    // check for whitespaces in key
    size_t whitespace = key.find_first_of(" \t");
    if (whitespace != std::string::npos)
        throw std::runtime_error("400 bad request"); //change to an actual response

    // remove leading and trailing whitespaces from value
    size_t start = value.find_first_not_of(" \t");
    size_t end = value.find_last_not_of(" \t");
    if (start != std::string::npos)
        value = value.substr(start, end - start + 1);
    else // only whitespace
        throw std::runtime_error("400 bad request"); //change to an actual response
}

// void Client::validateHeader()
// {
//     //TODO: Check how nginx reacts if there is more than single space between request line fields. RFC 7230 determines there should be only 1
//     //TODO: Check what if multiple headers have same key. std::map will overwrite them, so 1 remains, but what nginx does?
//     //TODO: Check that method and version are valid and no typos

//     //check if request line is valid
//     if (this->request.method.empty() || this->request.path.empty() || this->request.version.empty())
//         throw std::runtime_error("400 bad request"); //change to an actual response

//     // if HTTP/1.1 must have host header
//     if (this->request.version == "HTTP/1.1")
//     {
//         auto it = this->request.headers.find("Host");
//         if (it == this->request.headers.end())
//             throw std::runtime_error("400 bad request"); //change to an actual response
//     }
//     //if method is POST, check if transfer-encoding exist. if so, it must be chunked and content-length must not exist
//     if (this->request.method == "POST")
//     {
//         bool transferEncoding = false;
//         bool contentLength = false;

//         auto it = this->request.headers.find("Transfer-encoding");
//         if (it != this->request.headers.end())
//         {
//             transferEncoding = true;
//             if (it->second != "chunked")
//                 throw std::runtime_error("400 bad request"); //change to an actual response
//         }

//         it = this->request.headers.find("Content-Length");
//         if (it != this->request.headers.end())
//         {
//             contentLength = true;
//             this->request.contentLen = stoi(it->second);
//         }

//         if ((transferEncoding == false && contentLength == false)
//             || (transferEncoding == true && contentLength == true))
//         {
//             throw std::runtime_error("400 bad request"); //change to an actual response
//         }
//     }
// }

reqTypes Client::getMethodEnum()
{
    if (request.method == "GET")
        return GET;
    if (request.method == "POST")
        return POST;
    if (request.method == "DELETE")
        return DELETE;
    return INVALID;
}