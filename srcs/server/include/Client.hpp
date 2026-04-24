#pragma once

#include <string>

class HTTPRequest;
class HTTPResponse;

class Client{
    private:

    int fd;
    int server_fd;
    std::string buffer;

    bool request_ready;
    bool response_ready;

    HTTPRequest* request;
    HTTPResponse* response;

    Client();
    Client(int fd, int server_fd);
    ~Client();

    void appendData(const char* data, int size);
    bool hasCompleteRequest() const;
    void reset();
};