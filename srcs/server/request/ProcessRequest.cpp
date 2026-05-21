#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"

void ServerMaster::handleClient(int fd, size_t &idx)
{
    char buffer[4096];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    Router router;

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
    std::string root = config->root_path;
    std::string index = config->index;
    std::string filePath = router.routeRequest(req, config);

    // Получаем результат из роутера
    std::string statusCode = router.getStatusCode();
    std::string statusReason = router.getStatusReason(); // Если нужно для response.build

    // If request failed, try to serve a configured error page for the status code.
    if (statusCode != "200")
    {
        std::string errorPath = router.getErrorPagePath(config, statusCode);
        if (!errorPath.empty())
        {
            std::string errorFilePath = router.buildFilePath(config->root_path, errorPath, config->index);
            if (!errorFilePath.empty())
                filePath = errorFilePath;
        }
    }

    // ФОРМИРОВАНИЕ ОТВЕТА
    HTTPResponse response;
    std::string statusLine = statusCode;
    if (!statusReason.empty())
        statusLine += " " + statusReason;
    client.setResponse(response.build(req, filePath, statusLine));
    client.setState(Client::WRITING);
    fds[idx].events = POLLIN | POLLOUT;
}

void Client::processRequest()
{
    _req = HTTPRequest(_buffer);    
    _req.parse(); //make parser
    resetBytesSent();
}
