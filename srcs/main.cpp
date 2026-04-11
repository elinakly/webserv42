#include "ServerNode.hpp"
#include "ConfigParser.hpp"

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::cerr << "Usage: ./webserv <config_file>\n";
        return 1;
    }
    try
    {
        std::string config = (argc == 1 ? "config/config.conf" : argv[1]);

        ConfigParser parser;

        parser.parse(config);
        const std::vector<std::unique_ptr<ServerNode> >& servers = parser.getServers();
        // server.setup(servers);
        // server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}