#include "Client.hpp"


void Client::appendData(const char* data, int size)
{
    _buffer.append(data, size);
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

// void Client::setHTTPRequest(std::unique_ptr<HTTPRequest> request)
// { 
//     _HTTPRequest = std::move(request);
// }

// void Client::setHTTPResponse(std::unique_ptr<HTTPResponse> response) 
// { 
//     _HTTPResponse= std::move(response); 
// }