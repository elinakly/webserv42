#pragma once

#include <string>
#include <memory>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerMaster.hpp"

class HTTPRequest;
class HTTPResponse;


class Client {
    public:
        enum State {
            READING,
            WRITING,
            DONE
        };

    private:
        int _fd;
        int _server_fd;

        std::string _buffer;
        HTTPRequest _req;
        std::string _response;
        size_t _bytes_sent;

        State _state;
        // std::unique_ptr<HTTPRequest> _HTTPRequest;
        // std::unique_ptr<HTTPResponse> _HTTPResponse;
    public:
        Client(int fd, int server_fd)
            : _fd(fd), _server_fd(server_fd), _buffer(), _req(), _response(), _bytes_sent(0), _state(READING) {}
        ~Client() = default;
        
        const std::string& getResponse() const { return _response; }
        const std::string& getBuffer() const {return _buffer; }
        HTTPRequest& getRequest()  { return _req; }
        size_t getBytesSent() const { return _bytes_sent; }
        State getState() const { return _state; }
        int getServerFd() const { return _server_fd; }

        void setState (State state);
        void setResponse(const std::string& res) { _response = res; }
        
        void appendData(const char* data, int size);
        bool hasCompleteRequest() const;
        void processRequest();
        void addBytesSent(size_t bytes);
        void resetBytesSent();
};