#include "ServerMaster.hpp"

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

void ServerMaster::runServers()
{
    // add server sockets
    initPoll();

    while (true)
    {
        pollLoop();
    }
}