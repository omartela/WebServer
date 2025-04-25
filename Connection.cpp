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

// Connection::Connection(const Connection& copy) 
// {
//     this->fd = copy.fd;
//     this->state = copy.state;
//     this->read_buffer = copy.read_buffer;
//     this->write_buffer = copy.write_buffer;
//     this->bytes_read = copy.bytes_read;
//     this->bytes_written = copy.bytes_written;
//     nConnections++;
// }

// Connection& Connection::operator=(const Connection& copy)
// {
//     if (this != &copy)
//     {
//         this->fd = copy.fd;
//         this->state = copy.state;
//         this->read_buffer = copy.read_buffer;
//         this->write_buffer = copy.write_buffer;
//         this->bytes_read = copy.bytes_read;
//         this->bytes_written = copy.bytes_written;
//         nConnections++;
//     }
//     return *this;
// }

void Connection::reset()
{
    this->state = VACANT;
    this->read_buffer.clear();
    this->write_buffer.clear();
    this->bytes_read = 0;
    this->bytes_written = 0;
    nActiveConnections--;
}
