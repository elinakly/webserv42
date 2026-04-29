#include "HttpRequest.hpp"

bool HTTPRequest::parse()
{
    _method = "GET";
    _path = "/";
    _version = "HTTP/1.1";
    _headers["Connection"] = "close";
    _headers["Content-Type"] = "text/plain";
    _headers["Host"] = "127.0.0.1:8000";
    // headers["Content-Length:"] = ""
    return true;
}