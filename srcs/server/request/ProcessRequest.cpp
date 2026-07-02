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
    HTTPRequest& req = client.getRequest(); // Достаем запрос из клиента

    // Отделяем query string от пути для поиска location
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
    // Если это не CGI файл, вернуть false чтобы обработать как статический файл
    if (cgiMap.find(extension) == cgiMap.end())
        return false;
    // Это CGI файл - обработаем его
    try
    {
        CgiHandler cgiHandler(req, *location);
        std::string cgiOutput = cgiHandler.execute(); 
        // Формируем ответ с выводом CGI скрипта
        HTTPResponse response;
        std::string statusLine = "200 OK"; // Для успешного скрипта статус всегда 200 OK
        client.setResponse(response.buildCgiResponse(req, statusLine, cgiOutput));
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "CGI Runtime Error: " << e.what() << std::endl;
        // Отправляем 500 ошибку при ошибке выполнения CGI
        HTTPResponse response;
        std::string errorPath = router.buildErrorResponsePath(config, "500");
        client.setResponse(response.build(req, errorPath, "500 Internal Server Error"));
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
        return true; // Возвращаем true, так как ошибку мы уже обработали и записали в клиента
    }
}
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
