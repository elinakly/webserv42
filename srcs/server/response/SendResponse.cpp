#include "ServerMaster.hpp"

void ServerMaster::sendResponse(int fd)
{
    std::map<int, std::unique_ptr<Client>>::iterator it = clients.find(fd);
    if (it == clients.end() || !it->second)
        return;

    Client &client = *it->second;

    if (client.getState() != Client::WRITING)
        return;
    
    size_t bytes_sent = client.getBytesSent();
    const std::string &response = client.getResponse();
    size_t total = client.getResponse().size();

    int bytes = send(fd, response.c_str() + bytes_sent, total - bytes_sent, 0); //send to client
    if(bytes <= 0)
    {
        client.setState(Client::DONE);
        return;
    }

    client.addBytesSent(bytes);

    if (client.getBytesSent() >= total)
        client.setState(Client::DONE);
}