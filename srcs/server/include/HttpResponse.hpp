#pragma once

#include <string>
#include "HttpRequest.hpp"
#include <fstream>
#include "iostream"

class HTTPResponse 
{
    private:
        std::string _status;
        std::string _version;
        std::string _reasonPhrase;
        std::string _body;
        std::string _contentType;
        std::string _contentLength;
        std::string _path;
        std::string _connection;

    public:
        HTTPResponse() = default;
        ~HTTPResponse() = default;

        std::string getStatus() const { return _status; }

        std::string build(const HTTPRequest& request);
        std::string buildStatusLine(const HTTPRequest& request);
        std::string buildHeaders();
        std::string buildBody();
};