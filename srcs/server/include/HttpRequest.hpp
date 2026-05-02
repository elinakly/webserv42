#pragma once

#include <string>
#include <map>

class HTTPRequest {
public:
    std::string _method;
    std::string _path;
    std::string _version;
    std::string _status_reason;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _rawRequest;

    HTTPRequest(std::string& rawRequest) : _rawRequest(rawRequest) {};
    HTTPRequest();
    ~HTTPRequest() = default;

    std::string getPath() {return _path;}
    bool parse();
};