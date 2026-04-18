#pragma once

class Socket
{
    private:
        int _fd;
    public:
        Socket(int fd) : _fd(fd) {}
        ~Socket();

        const int& getFd() { return _fd; }
        


};