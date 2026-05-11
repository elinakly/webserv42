#include "ServerMaster.hpp"

void ServerMaster::handleClient(int fd, size_t &idx)
{
    char buffer[4096];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0); //read date from client
    if (bytes <= 0)
    {
        if (bytes == 0)
            std::cout << "Client disconnected: " << fd << std::endl;
        else
            perror("recv");

        cleanUp(fd, idx);
        return;
    }
    std::map<int, std::unique_ptr<Client>>::iterator it = clients.find(fd);
    if (it == clients.end() || !it->second) //if no client
    {
        cleanUp(fd, idx);
        return;
    }
    Client &client = *it->second; //client& = std::unique_ptr<Client>>
    client.appendData(buffer, bytes);
    if (client.hasCompleteRequest())
    {
        client.processRequest();

        HTTPRequest &req = client.getRequest();
        Server *config = listenSockets[client.get]
        std::string filePath = req.getPath();
        std::string status = req.getStatusReason();

        HTTPResponse response;
        client.setResponse(response.build(req, filePath, status));
        
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
    }
}

void Client::processRequest()
{
    _req = HTTPRequest(_buffer);    
    _req.parse(); //make parser
    resetBytesSent();
}
