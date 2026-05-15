#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"


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
