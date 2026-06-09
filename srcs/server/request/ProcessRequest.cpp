#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"
#include "HttpRequest.hpp"
#include "CgiHandler.hpp"

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
    bool parseOk = client.processRequest();
    HTTPRequest &req = client.getRequest();
    Server *config = listenSockets[client.getServerFd()];
    std::string root = config->root_path;
    std::string index = config->index;

    // If parsing failed, respond with 400 before routing.
    if (!parseOk)
    { 
        std::string statusCode = "400";
        std::string statusReason = "Bad Request";
        std::string filePath = router.buildErrorResponsePath(config, statusCode);

        HTTPResponse response;
        std::string statusLine = statusCode + " " + statusReason;
        client.setResponse(response.build(req, filePath, statusLine));
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
        return;
    }

    std::string filePath = router.routeRequest(req, config);

    // Получаем результат из роутера
    std::string statusCode = router.getStatusCode();
    std::string statusReason = router.getStatusReason(); // Если нужно для response.build

    // If request failed, try to serve a configured error page for the status code.
    if (statusCode != "200")
        filePath = router.buildErrorResponsePath(config, statusCode);
    // ФОРМИРОВАНИЕ ОТВЕТА
    HTTPResponse response;
    std::string extraHeaders;
    if (statusCode == "302" && !router.getRedirectPath().empty())
        extraHeaders = "Location: " + router.getRedirectPath() + "\r\n";
    std::string statusLine = statusCode;
    if (!statusReason.empty())
        statusLine += " " + statusReason;
    client.setResponse(response.build(req, filePath, statusLine, extraHeaders));
    client.setState(Client::WRITING);
    fds[idx].events = POLLIN | POLLOUT;
}

bool Client::processRequest()
{
    _req = HTTPRequest(_buffer);    
    bool ok = _req.parse(); // make parser
    resetBytesSent();
    return ok;
}
