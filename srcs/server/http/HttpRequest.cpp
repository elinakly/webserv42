#include "HttpRequest.hpp"

#include <sstream>

bool HTTPRequest::parse()
{
    std::istringstream requestStream(_rawRequest);
    std::string requestLine;

    if (!std::getline(requestStream, requestLine))
        return false;

    if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r')
        requestLine.erase(requestLine.size() - 1);

    std::istringstream lineStream(requestLine);
    if (!(lineStream >> _method >> _path >> _version))
        return false;

    return true;
}