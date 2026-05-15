#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"

std::string ServerMaster::findLocationRoot(const LocationNode * location, Server *config)
{
    std::string root = config->root_path;
    if (!location)
        return (root);
    for (const auto& directives : location->getDirectives())
    {
        if (const RootNode* node = dynamic_cast<const RootNode*>(directives.get()))
            return(node->getPath());
    }
    return(root);
}
std::string ServerMaster::findLocationIndex(const LocationNode *location, Server *config)
{
    std::string index = config->index;
    if (!location)
        return(index);
    for (const auto & directives : location->getDirectives())
    {
        if (const IndexNode *index = dynamic_cast<const IndexNode*>(directives.get()))
            return(index->getPath());
    }
    return(index);
}
std::string ServerMaster::buildFilePath(const std::string &root, const std::string &requestPath, std::string &index, std::string status)
{
    std::string filePath = root + requestPath;
    struct stat PathStats;
    if (stat(filePath.c_str(), &PathStats) != 0)
    {
        status = "404 Not Found";
        return (filePath);
    }
    if (S_ISDIR(PathStats.st_mode))
    {
        if (filePath.back() != '/')
            filePath += '/';
        filePath += index;
        if (stat(filePath.c_str(), &PathStats)!= 0)
            status = "404 Not Found";
    }
    return (filePath);
}
std::string ServerMaster::handleErrorPage(Server *config, const std::string &root)
{
    std::map<int, std::string>::const_iterator it = config->errors.find(404);
    if (it != config->errors.end())
        return(root + it->second);
    return("www/errors/404_not_found.html");
}
void ServerMaster::handleClient(int fd, size_t &idx)
{
    char buffer[4096];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0)
    {
        if (bytes == 0)
            std::cout << "Client disconnected: " << fd << std::endl;
        else
            perror("recv");

        cleanUp(fd, idx);
        return;
    }
    auto it = clients.find(fd);
    if (it == clients.end() || !it->second)
    {
        cleanUp(fd, idx);
        return;
    }
    Client &client = *it->second;
    client.appendData(buffer, bytes);

    if (!client.hasCompleteRequest())
        return;

    client.processRequest();
    HTTPRequest &req = client.getRequest();
    Server *config = listenSockets[client.getServerFd()];
    const LocationNode *location =
        findBestLocation(*config, req.getPath());
    std::string status = "200 OK";
    // 1. ROOT
    std::string root =
        findLocationRoot(location, config);
    // 2. INDEX
    std::string index =
        findLocationIndex(location, config);
    // 3. BUILD PATH
    std::string filePath =
        buildFilePath(root, req.getPath(), index, status);
    // 4. ERROR PAGE
    if (status != "200 OK")
    {
        filePath = handleErrorPage(config, root);
    }
    HTTPResponse response;
    client.setResponse(response.build(req, filePath, status));
    client.setState(Client::WRITING);
    fds[idx].events = POLLIN | POLLOUT;
}

void Client::processRequest()
{
    _req = HTTPRequest(_buffer);    
    _req.parse(); //make parser
    resetBytesSent();
}
