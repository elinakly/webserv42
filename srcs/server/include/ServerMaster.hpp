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
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#include "LocationNode.hpp"
#include "ServerNode.hpp"
#include "Client.hpp"

class ServerNode;
class Client;

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


const LocationNode*	findBestLocation(const Server &server, const std::string & requestPath);

class ServerMaster
{
    private:
        std::vector<Server> _servers;
        std::map<int, Server*> listenSockets; 
        std::map<int, std::unique_ptr<Client>> clients;
        std::vector<struct pollfd> fds;
        static volatile sig_atomic_t _running;
        
    public:
        ServerMaster() = default;
        ~ServerMaster();

        static void handler(int signal);

        void setupServers(const std::vector<std::unique_ptr<ServerNode>>& servers);
        void initSockets();
        void runServers();
        void initPoll();
        void pollLoop();
        void dispatch(struct pollfd &pfd, size_t &idx);
        void handleAccept(int fd);
        void addClient(int newfd, int fd);
        void handleClient(int fd, size_t &idx);
        void sendResponse(int fd);
        void cleanUp(int fd, size_t &idx);
        // void shutdownServer();
        std::string findLocationRoot(const LocationNode * location, Server *config);
        std::string findLocationIndex(const LocationNode *location, Server *config);
        std::string buildFilePath(const std::string &root, const std::string &requestPath, std::string &index, std::string status);
        std::string handleErrorPage(Server *config, const std::string &root);

};

