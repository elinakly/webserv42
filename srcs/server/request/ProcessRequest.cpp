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
        client.setState(Client::WRITING);
        fds[idx].events = POLLIN | POLLOUT;
    }
}

void Client::processRequest()
{
    HTTPRequest request(_buffer);
    request.parse();

    HTTPResponse response;
    _response = response.build(request);

    // std::string body = "Hello 42\n";
    // _response =
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "Content-Length: " + std::to_string(body.size()) + "\r\n"
    //     "Connection: close\r\n"
    //     "\r\n" +
    //     body;

    resetBytesSent();
}
