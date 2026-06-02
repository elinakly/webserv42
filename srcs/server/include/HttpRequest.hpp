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
    std::string _uri;

    HTTPRequest(std::string& rawRequest) : _rawRequest(rawRequest) {};
    HTTPRequest();
    ~HTTPRequest() = default;

    const std::string getPath() const{return _path;}
    const std::string getBody() const{return (_body);};
    const std::string getMethod() const {return (_method);};
    const std::string getUri()const {return(_uri);};
    
    bool parse();
};