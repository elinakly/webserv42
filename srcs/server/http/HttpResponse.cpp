#include "HttpResponse.hpp"

std::string HTTPResponse::getMimeTypes(const std::string& path)
{
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") 
        return "text/html; charset=utf-8"; 
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") 
        return "text/css; charset=utf-8"; 
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") 
        return "image/png"; 
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg") 
        return "image/jpeg";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".ico") //special image file format used for website icons (favicons)
        return "image/x-icon"; 
    return "application/octet-stream"; //default mime type
}


std::string HTTPResponse::buildStatusLine()
{
    return _version + " " + _status_reason + "\r\n";
}

std::string HTTPResponse::readFile(const std::string& filePath)
{
    std::ifstream file;
    file.open(filePath, std::ios::binary); //open file in binary format "for png,  etc"
    if (!file.is_open())
        return ("");
                //create one string starting from begining up to end (eof)
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string HTTPResponse::buildHeaders()
{
    _contentType = getMimeTypes(_path);
    _connection = "Connection: close";
    return ("Content-Type: " + _contentType + "\r\n" + _connection + "\r\n");
}

void HTTPResponse::buildBody()
{
    _body = readFile(_path);

    if (_body.empty())
        _contentLength = "Content-Length: 0\r\n";
    else 
        _contentLength = "Content-Length: " + std::to_string(_body.size()) + "\r\n";
}


std::string HTTPResponse::build(const HTTPRequest& request, const std::string &path, const std::string& status)
{
    std::string response;

    _version = request.getVersion();
    _path = path;
    _status_reason = status;

    buildBody();
    std::string status_line = buildStatusLine();
    response += status_line;
    std::string headers = buildHeaders();
    response += headers;
    response += _contentLength;
    response += "\r\n";
    if (!_body.empty())
        response += _body;
    return (response);
}

// Overload for extra headers (for redirects)
std::string HTTPResponse::build(const HTTPRequest& request, const std::string &path, const std::string& status, const std::string &extraHeaders)
{
    std::string response;

    _version = request.getVersion();
    _path = path;
    _status_reason = status;

    buildBody();
    std::string status_line = buildStatusLine();
    response += status_line;
    std::string headers = buildHeaders();
    if (!extraHeaders.empty())
        headers += extraHeaders;
    response += headers;
    response += _contentLength;
    response += "\r\n";
    if (!_body.empty())
        response += _body;
    return (response);
}

// Build response for CGI output
std::string HTTPResponse::buildCgiResponse(const HTTPRequest& request, const std::string& status, const std::string& cgiOutput)
{
    std::string response;

    _version = request.getVersion();
    _status_reason = status;
    _body = cgiOutput;
    _contentType = "text/html; charset=utf-8";
    _connection = "Connection: close";

    std::string status_line = buildStatusLine();
    response += status_line;
    response += ("Content-Type: " + _contentType + "\r\n");
    response += _connection + "\r\n";
    response += ("Content-Length: " + std::to_string(_body.size()) + "\r\n");
    response += "\r\n";
    if (!_body.empty())
        response += _body;
    return (response);
}