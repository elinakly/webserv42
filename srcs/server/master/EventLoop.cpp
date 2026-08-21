#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"

void ServerMaster::initPoll()
{
    for (std::map<int, Server*>::const_iterator it = listenSockets.begin();
         it != listenSockets.end(); ++it)
    {
        struct pollfd pfd; 
        pfd.fd = it->first;
        pfd.events = POLLIN; // fill  the pollfd struct
        pfd.revents = 0;    
        fds.push_back(pfd);
    }
}

void ServerMaster::cleanUp(int fd, size_t &idx)
{
    if (fd >= 0)
        close(fd);

    clients.erase(fd);

    fds.erase(fds.begin() + idx);

    if (idx > 0)
        idx--;
    else
        idx = static_cast<size_t>(-1);
}

void ServerMaster::dispatch(struct pollfd pfd, size_t &idx)
{
    int fd = pfd.fd; //get fd
    
    if (fd == -1)
        return;
    if (_cgiProcesses.find(fd) != _cgiProcesses.end())
    {
        if (pfd.revents & (POLLIN | POLLHUP | POLLERR))
            handleCgiOutput(fd);
        return ;
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) // if error | disconected | wrong fd
    {
        if (!listenSockets.count(fd)) //if its not server socket (but client)
            cleanUp(fd, idx);
        return;
    }
    if (listenSockets.count(fd)) //if server socket
    {
        if (pfd.revents & POLLIN)
            handleAccept(fd); //add to waitlist
        return;
    }
    auto it = clients.find(fd);
    if (it == clients.end())
        return;
    Client &client = *it->second;

    if (pfd.revents & POLLIN && (client.getState() == Client::READING)) //read (recv())
        handleClient(fd, idx);
        
    if (pfd.revents & POLLOUT && (client.getState() == Client::WRITING)) 
    {
        sendResponse(fd);   //write (send())
        if (client.getState() == Client::DONE)
        {   
            cleanUp(fd, idx);
            return;
        }
    }
}
void ServerMaster::pollLoop()
{
    if (fds.empty())
        return;
    int ready = poll(fds.data(), fds.size(), 1000);

    if (ready < 0)
    {
        if (errno == EINTR)
            return;
        perror("poll");
        return;
    }
    for (size_t i = 0; i < fds.size(); i++)
    {
        auto it = clients.find(fds[i].fd);
        if (it == clients.end())
            continue;
        //if this fd is not from HTTP client then we're skiping it
        Client &client = *it->second;

        if (client.getState() == Client::READING)
        {
            if (time(NULL) - client._lastActivity > 30)
            {
                HTTPRequest &req = client.getRequest();
                HTTPResponse response;
                Router router;

                Server *config = listenSockets[client.getServerFd()];
                std::string errorPath =router.buildErrorResponsePath(config, "408");
                client.setResponse(response.build(req,errorPath,"408 Request Timeout"));
                client.setState(Client::WRITING);
                fds[i].events = POLLOUT;
            }
        }
        //checking for the timeout
    }
    //CGI timeout
    for (std::map<int, CgiProcess>::iterator it = _cgiProcesses.begin();
        it != _cgiProcesses.end();)
    {
        CgiProcess &process = it->second;

        if (time(NULL) - process.startTime >= 10)
        {
            kill(process.pid, SIGKILL);
            waitpid(process.pid, NULL, 0);

            process.handler->releasePipeOut();

            close(process.pipeFd);

            for (size_t i = 0; i < fds.size(); i++)
            {
                if (fds[i].fd == process.pipeFd)
                {
                    fds.erase(fds.begin() + i);
                    break;
                }
            }
            std::map<int, std::unique_ptr<Client> >::iterator clientIt;
            clientIt = clients.find(process.clientFd);

            if (clientIt != clients.end())
            {
                Client &client = *clientIt->second;
                HTTPRequest &req = client.getRequest();

                Router router;
                HTTPResponse response;
                Server *config;
                config = listenSockets[client.getServerFd()];
                std::string errorPath;
                errorPath = router.buildErrorResponsePath(config, "504");

                client.setResponse(response.build(req,errorPath,"504 Gateway Timeout"));
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
            it = _cgiProcesses.erase(it);
        }
        else
            ++it;
    }
    for (size_t idx = 0; idx < fds.size() && ready > 0; idx++)
    {
        short re = fds[idx].revents;
        if (re == 0)
            continue;
        ready--;
        dispatch(fds[idx], idx);
    }
}


void ServerMaster::addClient(int newfd, int fd)
{
    struct pollfd clientPfd; //fill pollfd struct for client
    clientPfd.fd = newfd; 
    clientPfd.events = POLLIN;
    clientPfd.revents = 0;
    fds.push_back(clientPfd);
    clients[newfd] = std::unique_ptr<Client>(new Client(newfd, fd));
}

void ServerMaster::handleAccept(int fd)
{
    while (true)
    {
        int newfd = accept(fd, NULL, NULL);

        if (newfd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("accept");
            break;
        }
        int flags = fcntl(newfd, F_GETFL, 0);
        if (flags == -1 || fcntl(newfd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            close(newfd);
            perror("fcntl");
            continue;
        }
        addClient(newfd, fd);
    }
}
