#include "Connection.hpp"

Connection::Connection()
{
    this->fd = nConnections;
    this->state = VACANT;
    this->read_buffer.clear();
    this->write_buffer.clear();
    this->bytes_read = 0;
    this->bytes_written = 0;
    nConnections++;
}

Connection::Connection(int newFd)
{
    this->fd = newFd;
    this->state = VACANT;
    this->read_buffer.clear();
    this->write_buffer.clear();
    this->bytes_read = 0;
    this->bytes_written = 0;
    nConnections++;
}

void Connection::reset()
{
    this->state = VACANT;
    this->read_buffer.clear();
    this->write_buffer.clear();
    this->bytes_read = 0;
    this->bytes_written = 0;
    nActiveConnections--;
}
