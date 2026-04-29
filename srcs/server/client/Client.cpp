#include "Client.hpp"


void Client::appendData(const char* data, int size)
{
    _buffer.append(data, size);
}

void Client::processRequest()
{
    std::string body = "Hello from webserv42\n";

    _response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    resetBytesSent();
}

bool Client::hasCompleteRequest() const
{
    return _buffer.find("\r\n\r\n") != std::string::npos;
}
void Client::addBytesSent(size_t bytes)
{ 
    _bytes_sent += bytes;
}

void Client::resetBytesSent()
{ 
    _bytes_sent = 0;
}

void Client::setState (State state)
{
    _state = state;
}