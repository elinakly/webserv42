#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <cstring>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "LocationNode.hpp"
#include "ServerNode.hpp"

class ServerNode;

struct Server{
    int port;
    int max_body_size;
    std::string index;
    std::string host;
    std::string root_path;
    std::string server_name;
    std::map<int, std::string> errors;
    std::vector<std::string> methods;
    std::vector<const LocationNode*> locations;
};

class ServerMaster
{
    private:
        std::vector<Server> _servers;
        std::map<int, Server*> listenSockets; 
        std::map<int, int> clientToServer;
        
    public:
        ServerMaster() = default;
        ~ServerMaster();

        void setupServers(const std::vector<std::unique_ptr<ServerNode>>& servers);
        void initSockets();
        void runServers();
        // void initPoll();


};

