#include "ServerMaster.hpp"
#include <netdb.h>
#include <fcntl.h>

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
    initSockets();
}

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

void ServerMaster::runServers()
{
    std::vector<struct pollfd> fds;

    // add server sockets
    for (std::map<int, Server*>::const_iterator it = listenSockets.begin();
         it != listenSockets.end(); ++it)
    {
        struct pollfd pfd; 
        pfd.fd = it->first;
        pfd.events = POLLIN; // fill  the pollfd struct
        pfd.revents = 0;    
        fds.push_back(pfd);
    }

    while (true)
    {
        if (fds.empty())
            break;

        int ready = poll(fds.data(), fds.size(), -1); // number of ready sockets 
        if (ready < 0)
        {
            perror("poll");
            break;
        }

        for (size_t idx = 0; idx < fds.size() && ready > 0; idx++) // checking only active sockets
        {
            short re = fds[idx].revents; //set the type of event that is occured

            if (re == 0) //if no activity we skip
                continue;
            ready--; // dicrease the number of ready sockets
            int fd = fds[idx].fd; //get fd
            if (re & (POLLERR | POLLHUP | POLLNVAL)) // if error | disconected | wrong fd
            {
                if (!listenSockets.count(fd)) //if its not server socket (but client)
                {
                    close(fd); 
                    clientToServer.erase(fd); //remove from map of fds
                    fds.erase(fds.begin() + idx); // remove from pollfd struct
                    idx--; //come back to not to lose next fd
                }
                continue;
            }
            if (!(re & POLLIN)) // if not ready to recv() -- skip
                continue;
            if (listenSockets.count(fd)) // if server socket
            {
                while (true)
                {
                    int newfd = accept(fd, NULL, NULL); //accept client
                    if (newfd < 0)
                    {  //no more clients
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        perror("accept");
                        break;
                    }

                    struct pollfd clientPfd; //fill pollfd struct for client
                    clientPfd.fd = newfd; 
                    clientPfd.events = POLLIN;
                    clientPfd.revents = 0;
                    fds.push_back(clientPfd);
                    clientToServer[newfd] = fd; //client -> server

                    std::cout << "Client connected: " << newfd << std::endl;
                }
                continue; // next socket
            }
            char buffer[4096];
            int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0); //read date from client
            if (bytes <= 0)
            {
                if (bytes == 0)
                    std::cout << "Client disconnected: " << fd << std::endl;
                else
                    perror("recv");

                close(fd);
                clientToServer.erase(fd);
                fds.erase(fds.begin() + idx);
                idx--;
                continue;
            }
            buffer[bytes] = '\0'; //make string
            const std::string body = "Hello from webserv42\n"; //responce
            std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: " + std::to_string(body.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" +
                body; //http responce
            if (send(fd, response.c_str(), response.size(), 0) == -1) //send to client
                perror("send");
            close(fd); //close connection
            clientToServer.erase(fd);
            fds.erase(fds.begin() + idx);
            idx--;
        }
    }
}