#include "ServerMaster.hpp"
#include <netdb.h>

ServerMaster::~ServerMaster()
{
    
}

void ServerMaster::setupServers(const std::vector<std::unique_ptr<ServerNode>>& servers)
{
    _servers.clear();
    _servers.reserve(servers.size());

    for(size_t i = 0; i < servers.size(); i++)
    {
        Server srv;

        srv.port = servers[i]->getPort();
        srv.max_body_size = servers[i]->getMax_body_size();
        srv.host = servers[i]->getHost();
        srv.server_name = servers[i]->getServer_name();
        srv.root_path = servers[i]->getRoot();
        srv.index = servers[i]->getIndex_path();
        srv.methods = servers[i]->getMethods();
        srv.errors = servers[i]->getErrors();

        const auto& locations = servers[i]->getLocations();
        for (size_t j = 0; j < locations.size(); j++)
        {
            if (const LocationNode* location = dynamic_cast<const LocationNode*>(locations[j].get()))
                srv.locations.push_back(location);
        }
        _servers.push_back(srv);
    }
    // initSockets();
}

void ServerMaster::initSockets()
{
    struct addrinfo hints;
    struct addrinfo* res, *p;
    int sock_fd;
    for(size_t i = 0; i < _servers.size(); i++)
    {
        int port = _servers[i].port;
        std::string host = _servers[i].host;
        if (listenSockets.count(port)) //eliminate repetitions 
            continue;

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
            if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) //make adress reusable
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

        listenSockets[port] = sock_fd;
    }
}

void ServerMaster::runServers()
{
    initSockets();

    while (true)
        sleep(1);
}