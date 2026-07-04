#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"
#include "HttpRequest.hpp"
#include "CgiHandler.hpp"

#include <cerrno>

bool ServerMaster::handleCgiRequest(Server* config, Client& client, const std::string& filePath, size_t& idx)
{
    Router router;
    HTTPRequest& req = client.getRequest();

    std::string requestPath = req.getPath();
    size_t queryPos = requestPath.find("?");
    if (queryPos != std::string::npos)
        requestPath = requestPath.substr(0, queryPos);

    const LocationNode* location = router.findBestLocation(*config, requestPath);
    if (!location)
        return false;

    const std::map<std::string, std::string>& cgiMap = location->getCgi();

    std::string extension;
    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos)
        extension = filePath.substr(dotPos);

    // Не CGI — продолжаем обычную обработку
    if (cgiMap.find(extension) == cgiMap.end())
        return false;

    try
    {
        CgiHandler cgiHandler(req, *location);
        std::string cgiOutput = cgiHandler.execute();

        HTTPResponse response;
        client.setResponse(response.buildCgiResponse(req, "200 OK", cgiOutput));
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "CGI Runtime Error: " << e.what() << std::endl;

        HTTPResponse response;

        std::string statusCode = "500";
        std::string statusLine = "500 Internal Server Error";

        if (std::string(e.what()) == "CGI timeout")
        {
            statusCode = "504";
            statusLine = "504 Gateway Timeout";
        }

        std::string errorPath = router.buildErrorResponsePath(config, statusCode);

        client.setResponse(response.build(req, errorPath, statusLine));
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
        return true;
    }
}
void ServerMaster::handleClient(int fd, size_t &idx)
{
    char buffer[4096];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    Router router;

    if (bytes <= 0)
    {
        if (bytes < 0)
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

    if (statusCode == "200" && req.getMethod() == "DELETE")
    {
        if (!filePath.empty() && unlink(filePath.c_str()) == 0)
        {
            statusCode = "204";
            statusReason = "No Content";
            filePath.clear();
        }
        else if (!filePath.empty())
        {
            if (errno == ENOENT)
            {
                statusCode = "404";
                statusReason = "Not Found";
            }
            else if (errno == EACCES || errno == EPERM)
            {
                statusCode = "403";
                statusReason = "Forbidden";
            }
            else
            {
                statusCode = "500";
                statusReason = "Internal Server Error";
            }
            filePath = router.buildErrorResponsePath(config, statusCode);
        }
    }

    // If request failed, try to serve a configured error page for the status code.
    if (statusCode != "200" && statusCode != "204")
        filePath = router.buildErrorResponsePath(config, statusCode);
    // ОБРАБОТКА CGI ЗАПРОСОВ
    if (statusCode == "200" && !filePath.empty())
        if (handleCgiRequest(config, client, filePath, idx))
            return;
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
