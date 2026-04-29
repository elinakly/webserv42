#include "HttpResponse.hpp"

std::string HTTPResponse::buildStatusLine(const HTTPRequest& request)
{
    _version = request._version;
    _path = request._path;
    if (_path == "/")
    {
        _status = "200";
        _reasonPhrase = "OK";
    }
    else
    {
        _status = "404";
        _reasonPhrase = "Not Found";
    }
    return _version + " " + _status + " " + _reasonPhrase + "\r\n";
}

std::string HTTPResponse::buildHeaders()
{
    if(_status == "200" && _path == "/")
        _contentType = "text/html";
    else if(_status == "404")
        _contentType = "text/html";
    _connection = "Connection: close";
    return ("Content-Type: " + _contentType + "\r\n");
}

std::string HTTPResponse::buildBody()
{
    std::ifstream file;
    file.open("www/index.html");
    std::string buffer;
    std::string line;
    while (std::getline(file, line))
    {
        buffer += line;
        buffer += '\n';
    }
    _body = buffer;
    _contentLength = std::to_string(buffer.size());
    return(_body);
}


std::string HTTPResponse::build(const HTTPRequest& request)
{
    std::string response;

    std::string status = buildStatusLine(request);
    std::string headers = buildHeaders();
    std::string body = buildBody();

    response = status + headers + "Content-Length: " + _contentLength  + "\r\n" + _connection + "\r\n\r\n" + body;
    std::cout << response;
    return (response);

}