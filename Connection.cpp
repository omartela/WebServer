#include "Connection.hpp"

Connection::Connection()
{
    this->fd = -1;
    this->state = VACANT;
    this->readBuffer.reserve(1024);
    this->writeBuffer.reserve(1024);
    this->bytesRead = 0;
    this->bytesWritten = 0;
    nConnections++;
}

Connection::~Connection() {
    nConnections--;
};

Connection::Connection(const Connection& copy)
{
    *this = copy;
}

Connection& Connection::operator=(const Connection& copy)
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

void Connection::softReset()
{
    this->state = IDLE;
    this->readBuffer.clear();
    this->writeBuffer.clear();
    this->bytesRead = 0;
    this->bytesWritten = 0;
}

void Connection::hardReset()
{
    this->state = VACANT;
    this->readBuffer.clear();
    this->writeBuffer.clear();
    this->bytesRead = 0;
    this->bytesWritten = 0;
    nConnections--;
}