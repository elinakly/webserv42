#include "ServerMaster.hpp"

void ServerMaster::initSockets()
{
    struct addrinfo hints;
    struct addrinfo* res, *p;
    int sock_fd;
    std::set<int> usedPorts;
    for(size_t i = 0; i < _servers.size(); i++)
    {
        int port = _servers[i].port;
        std::string host = _servers[i].host;
        if (usedPorts.count(port))
            throw std::runtime_error(std::string("parser: Dublicate port detected"));
        usedPorts.insert(port);

        std::memset(&hints, 0, sizeof(hints)); //clear hints structure
        hints.ai_family = AF_INET; //adress family(only ipv4)
        hints.ai_socktype = SOCK_STREAM; //tcp socket

        std::string portStr = std::to_string(port);
        int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res); //resolve host + port into sockaddr
                                                                                // return a * to linked list of res
        if (status != 0)
            throw std::runtime_error(std::string("getaddrinfo failed: ") + gai_strerror(status));

        sock_fd = -1;
        for (p = res; p != NULL; p = p->ai_next)
        {
            sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); //get fd
            if (sock_fd < 0)
                continue;

            int opt = 1;
            //so_reusable kernel holds port in timewait
            if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) //make adress reusable
            {
                close(sock_fd);
                throw std::runtime_error(std::string("setsockopt failed: ") + strerror(errno));
            }

            if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == 0) //bind socket to resolved ip+port (socket becomes server)
                break;

            close(sock_fd);
            sock_fd = -1;
        }

        freeaddrinfo(res);
        if (sock_fd < 0)
            throw std::runtime_error(std::string("bind failed: ") + strerror(errno));

        if(listen(sock_fd, 128) == -1) //waitlist for connection
        {
            close(sock_fd);
            throw std::runtime_error(std::string("listen failed: ") + strerror(errno)); 
        }

        int flags = fcntl(sock_fd, F_GETFL, 0);
        if (flags == -1 || fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            close(sock_fd);
            throw std::runtime_error(std::string("fcntl failed: ") + strerror(errno));
        }

        listenSockets[sock_fd] = &_servers[i];
    }
}