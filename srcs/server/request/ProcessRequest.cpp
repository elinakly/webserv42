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
    //Getting HTTP from the current client
    std::string requestPath = req.getPath();
    size_t queryPos = requestPath.find("?");
    if (queryPos != std::string::npos)
        requestPath = requestPath.substr(0, queryPos);
    //Removing the query string
    const LocationNode* location = router.findBestLocation(*config, requestPath);
    if (!location)
        return false;
    //Tries to find the location for the CGI if there's no location found, then this is not CGI
    const std::map<std::string, std::string>& cgiMap = location->getCgi();
    //getting the CGI map
    std::string extension;
    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos)
        extension = filePath.substr(dotPos);
    //gets the extension
    if (cgiMap.find(extension) == cgiMap.end())
        return false;
    //If the file is not configured as CGI, continue with normal HTTP handling
    try
    {
        std::unique_ptr<CgiHandler> cgiHandler(new CgiHandler(req, *location));
        pid_t pid = cgiHandler->start();
        //starts the CGI
        cgiHandler->writeBody();
        int pipeFd = cgiHandler->getPipeFd();
        fcntl(pipeFd, F_SETFL, O_NONBLOCK);
        CgiProcess process;
        process.pid = pid;
        process.pipeFd = pipeFd;
        process.clientFd = fds[idx].fd;
        process.clientIdx = idx;
        process.startTime = time(NULL);
        process.output = "";
        process.server = config;
        //fils the information for the server to handle the CGI
        process.handler = std::move(cgiHandler);
        _cgiProcesses[pipeFd] = std::move(process);
        pollfd cgiPoll;
        cgiPoll.fd = pipeFd;
        cgiPoll.events = POLLIN;
        cgiPoll.revents = 0;
        fds.push_back(cgiPoll);
        fds[idx].events = 0;
        return (true);
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
    //if theres no client then closing the connection
    Client &client = *it->second;
    client.appendData(buffer, bytes);
    //getting the information about client 
    client._lastActivity = time(NULL);
    //updating the last activity time
    if (!client.hasCompleteRequest())
        return;
    //check for the request complete
    bool parseOk = client.processRequest();
    HTTPRequest &req = client.getRequest();
    Server *config = listenSockets[client.getServerFd()];
    //getting the config of the server
    std::string root = config->root_path;
    std::string index = config->index;

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
    // If parsing failed respond with 400 before routing
    std::string filePath = router.routeRequest(req, config);
    std::string statusCode = router.getStatusCode();
    std::string statusReason = router.getStatusReason();
    if (statusCode == "200" && !filePath.empty())
    {
        if (handleCgiRequest(config, client, filePath, idx))
            return;
    }
    //Checks if this request should be handled by CGI
    if (statusCode == "200" && req.getMethod() == "POST")
    {
        struct stat st;
        if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            if (filePath.back() != '/')
                filePath += '/';

            filePath += "upload.bin";
        }
        //Saves POST body as upload.bin inside the directory
        std::ofstream out(filePath.c_str(), std::ios::binary);

        if (!out)
        {
            statusCode = "500";
            statusReason = "Internal Server Error";
            filePath = router.buildErrorResponsePath(config, statusCode);
        }
        //if directory cannot be opened then its "500" error
        else
        {
            out.write(req.getBody().c_str(), req.getBody().size());
            out.close();

            statusCode = "201";
            statusReason = "Created";
        }
        //else just writing body
    }
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

    // If request failed, try to serve a configured error page for the status code
    // Redirects are handled separately and must not be converted into error pages
    if (statusCode[0] != '2' && statusCode[0] != '3')
        filePath = router.buildErrorResponsePath(config, statusCode);
    bool isDir = false;
    std::string dirBody;
    struct stat st;
    if (statusCode == "200" && !filePath.empty() && stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    {
        const LocationNode *location = router.findBestLocation(*config, req.getPath());
        if (location && location->getAutoIndex())
        {
            dirBody = router.generateDirectoryListing(filePath, req.getPath());
            isDir = true;
        }
    }
    //Check for the autoindex. If its on, then creates the HTML
    HTTPResponse response;
    std::string extraHeaders;

    if (statusCode == "302" && !router.getRedirectPath().empty())
        extraHeaders = "Location: " + router.getRedirectPath() + "\r\n";
    std::string statusLine = statusCode;

    if (!statusReason.empty())
        statusLine += " " + statusReason;
    if (isDir)
    {
        std::string rawResponse;

        rawResponse += "HTTP/1.1 200 OK\r\n";
        rawResponse += "Content-Type: text/html\r\n";
        rawResponse += "Content-Length: ";
        rawResponse += std::to_string(dirBody.size());
        rawResponse += "\r\n";
        rawResponse += "Connection: close\r\n";
        rawResponse += "\r\n";
        rawResponse += dirBody;
        client.setResponse(rawResponse);
    }
    //if directory requested then we're creating the HTTP response
    else
    {
        client.setResponse(
            response.build(req, filePath, statusLine, extraHeaders)
        );
    }
    //if requested path is not an directory then we're building a response
    client.setState(Client::WRITING);
    fds[idx].events = POLLIN | POLLOUT;
}

bool Client::processRequest()
{
    _req = HTTPRequest(_buffer);    
    bool ok = _req.parse();
    resetBytesSent();
    return ok;
}
void ServerMaster::handleCgiOutput(int pipeFd)
{
    std::map<int, CgiProcess>::iterator it;
    it = _cgiProcesses.find(pipeFd);

    if (it == _cgiProcesses.end())
        return;

    CgiProcess &process = it->second;

    char buff[4096];

    while (true)
    {
        ssize_t bytes = read(pipeFd, buff, sizeof(buff));

        if (bytes > 0)
        {
            process.output.append(buff, bytes);
            continue;
        }

        if (bytes == 0)
            break;

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        break;
    }

    int status;
    pid_t res = waitpid(process.pid, &status, WNOHANG);

    if (res == 0)
        return;

    if (res == -1)
    {
        perror("waitpid");
        return;
    }

    std::map<int, std::unique_ptr<Client> >::iterator clientIt;
    clientIt = clients.find(process.clientFd);

    if (clientIt != clients.end())
    {
        Client &client = *clientIt->second;
        HTTPRequest &req = client.getRequest();
        HTTPResponse response;

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            Router router;

            std::string errorPath;
            errorPath = router.buildErrorResponsePath(process.server, "500");

            client.setResponse(
                response.build(
                    req,
                    errorPath,
                    "500 Internal Server Error"
                )
            );
        }
        else
        {
            client.setResponse(
                response.buildCgiResponse(
                    req,
                    "200 OK",
                    process.output
                )
            );
        }

        client.setState(Client::WRITING);

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].fd == process.clientFd)
            {
                fds[i].events = POLLOUT;
                break;
            }
        }
    }

    process.handler->releasePipeOut();

    close(pipeFd);

    for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd == pipeFd)
        {
            fds.erase(fds.begin() + i);
            break;
        }
    }

    _cgiProcesses.erase(it);
}
