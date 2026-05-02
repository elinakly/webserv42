#include "HttpRequest.hpp"

#include <sstream>

static std::string sanitizeTargetToPath(const std::string& target)
{
    if (target.empty() || target[0] != '/')
        return "";

    std::string cleanTarget = target;

    size_t queryPos = cleanTarget.find('?');
    if (queryPos != std::string::npos)
        cleanTarget.erase(queryPos);

    if (cleanTarget.find("..") != std::string::npos)
        return "";

    if (cleanTarget == "/")
        return "./www/index.html";

    if (cleanTarget == "/")
        return "./www/index.html";
    if (cleanTarget == "/errors/404")
        return "./www/errors/404_not_found.html";

    return "./www" + cleanTarget;
}

HTTPRequest::HTTPRequest() : _rawRequest("") {}

bool HTTPRequest::parse()
{
    std::istringstream requestStream(_rawRequest);
    std::string requestLine;

    if (!std::getline(requestStream, requestLine))
        return false;

    if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r')
        requestLine.erase(requestLine.size() - 1);

    std::istringstream lineStream(requestLine);
    std::string target;
    if (!(lineStream >> _method >> target >> _version))
        return false;

    _path = sanitizeTargetToPath(target);
    if (_path.empty())
    {
        _status_reason = "400 Bad Request";
        _path = "./www/errors/400_bad_request.html";
        _version = "HTTP/1.1";
        return true;
    }

    if (_method != "GET")
    {
        _status_reason = "405 Method Not Allowed";
        _path = "./www/errors/405_method_not_allowed.html";
        _version = "HTTP/1.1";
        return true;
    }

    if (_version != "HTTP/1.1" && _version != "HTTP/1.0")
    {
        _status_reason = "400 Bad Request";
        _path = "./www/errors/400_bad_request.html";
        _version = "HTTP/1.1";
        return true;
    }

    _version = "HTTP/1.1";
    _status_reason = "200 OK";
    return true;
}