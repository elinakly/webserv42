#pragma once

#include <string>
#include <memory>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

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
        std::string _response;
        size_t _bytes_sent;

        State _state;
        // std::unique_ptr<HTTPRequest> _HTTPRequest;
        // std::unique_ptr<HTTPResponse> _HTTPResponse;
    public:
        Client(int fd, int server_fd)
            : _fd(fd), _server_fd(server_fd), _bytes_sent(0), _state(READING) {}
        ~Client() = default;

        const std::string& getResponse() const { return _response; }
        const std::string& getBuffer() const {return _buffer; }
        // HTTPRequest* getHTTPRequest() {return _HTTPRequest.get(); }
        // HTTPResponse* getHTTPResponse() {return _HTTPResponse.get(); }
        size_t getBytesSent() const { return _bytes_sent; }
        State getState() const { return _state; }

        void setState (State state);
        
        void appendData(const char* data, int size);
        bool hasCompleteRequest() const;
        void processRequest();
        void addBytesSent(size_t bytes);
        void resetBytesSent();
};