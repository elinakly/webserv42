#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "HttpRequest.hpp"

class HTTPResponse 
{
    private:
        std::string _status_reason;
        std::string _version;
        std::string _body;
        std::string _contentType;
        std::string _contentLength;
        std::string _path;
        std::string _connection;

    public:
        HTTPResponse() = default;
        ~HTTPResponse() = default;

        std::string getStatus() const { return _status_reason; }
        std::string build(const HTTPRequest& request, const std::string &path, const std::string& status);
        std::string buildStatusLine();
        std::string buildHeaders();
        void buildBody();
        std::string getMimeTypes(const std::string& path);
        std::string readFile(const std::string& filePath);


};