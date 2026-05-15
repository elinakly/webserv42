#include "Client.hpp"


void Client::appendData(const char* data, int size)
{
    _buffer.append(data, size);
}

bool Client::hasCompleteRequest() const
{
    //Searching for the end of the headers
    size_t  headerEnd = _buffer.find("\r\n\r\n");
    // if theres no headers, request is not full; waiting for another recv
    if (headerEnd == std::string::npos)
        return(false);
    //higlighting the header part
    std::string headers = _buffer.substr(0, headerEnd);
    //searching for the content length
    size_t pos = headers.find("Content-Length:");
    //if its an get request than response is ready
    //else if content length appeard, than its an post request
    if (pos == std::string::npos)
        return(true);
    //searching for the end of the header
    size_t end = headers.find("\r\n", pos);
    //if header is broken then we're protecting it
    if (end == std::string::npos)
        return (false);
    //Cutting the content legth
    std::string len = headers.substr(pos + 15, end -(pos + 15));
    int contentLegth = atoi(len.c_str());
    //searchin the start of the body
    size_t bodyStart = headerEnd + 4;
    //checking if the body is full
    return(_buffer.size() >= bodyStart + contentLegth);
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