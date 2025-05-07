#include "Client.hpp"

Client::Client()
{
    this->fd = -1;
    this->state = IDLE;
    this->readBuffer.reserve(1024);
    this->writeBuffer.reserve(1024);
    this->bytesRead = 0;
    this->bytesWritten = 0;
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
        this->state = copy.state;
        this->readBuffer = copy.readBuffer;
        this->writeBuffer = copy.writeBuffer;
        this->bytesRead = copy.bytesRead;
        this->bytesWritten = copy.bytesWritten;
    }
    return *this;
}

void Client::reset()
{
    this->state = IDLE;
    this->readBuffer.clear();
    this->writeBuffer.clear();
    this->bytesRead = 0;
    this->bytesWritten = 0;
    //clean httpRequest struct
}