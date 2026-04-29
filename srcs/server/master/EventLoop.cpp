#include "ServerMaster.hpp"


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
    if (clients.find(fd) != clients.end())
        std::cout << "Client disconnected: " << fd << std::endl;
    close(fd); 
    clients.erase(fd);
    fds.erase(fds.begin() + idx); // remove from pollfd struct
    if (idx > 0)
        idx--; //come back to not to lose next fd
    else
        idx = static_cast<size_t>(-1);
}

void ServerMaster::dispatch(struct pollfd &pfd, size_t &idx)
{
    int fd = pfd.fd; //get fd
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
    //std::map<int, std::unique_ptr<Client>>::iterator
    auto it = clients.find(fd);
    if (it == clients.end())
        return;
    Client &client = *clients[fd];

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

    int ready = poll(fds.data(), fds.size(), -1); // number of ready sockets 
    if (ready < 0)
    {
        perror("poll");
        return;
    }

    for (size_t idx = 0; idx < fds.size() && ready > 0; idx++) // checking only active sockets
    {
        short re = fds[idx].revents; //set the type of event that is occured

        if (re == 0) //if no activity we skip
            continue;
        ready--; // dicrease the number of ready sockets
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

    std::cout << "Client connected: " << newfd << std::endl;
}

void ServerMaster::handleAccept(int fd)
{
    while (true)
    {
        int newfd = accept(fd, NULL, NULL); //add  client to the waitlist
        if (newfd < 0)
        {  //no more clients
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
        }
        addClient(newfd, fd);
    }
}
