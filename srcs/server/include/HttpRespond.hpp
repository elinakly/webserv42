#pragma once

#include <string>

class HTTPResponse {
public:
    int status;
    std::string body;

    std::string raw;

    HTTPResponse();
    ~HTTPResponse();

    void build();
};