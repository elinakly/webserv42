#include "ServerNode.hpp"
#include "ListenNode.hpp"
#include "RootNode.hpp"
#include "ServerNameNode.hpp"
#include "MaxBodySizeNode.hpp"
#include "ErrorPageNode.hpp"
#include "AllowedMethodsNode.hpp"
#include "IndexNode.hpp"
#include "HostNode.hpp"


ServerNode::ServerNode() : _port(0), _max_body_size(0), _cgi()
{
}

void ServerNode::addDirective(std::unique_ptr<ASTNode> directive) noexcept
{
    _directives.push_back(std::move(directive));
}

void ServerNode::addLocation(std::unique_ptr<ASTNode> location) noexcept
{
    _locations.push_back(std::move(location));
}

#include "ConfigParser.hpp"

//This function os called when parser finds "CGI"
std::unique_ptr<ASTNode> ConfigParser::parseCgi()
{
	//creating a node container
    auto cgiNode = std::make_unique<CgiNode>();
	//reading a string per peers, while we are not found ";"
    while (peek().type == WORD)
    {
		//waiting for the ".py"
        std::string extension = expectWord();
		//waiting for "/usr/bin/python3"
        std::string path = expectWord();
		//adding it to the node
        cgiNode->addCgi(extension, path);
    }
	//at the end theres always supposed to be ";"
    expect(SEMICOLON);
    return cgiNode;
}

    }
    if (_port == 0)
        _port = 80;
    if (_root.empty())
        _root = "./www";
    if (_max_body_size == 0)
        _max_body_size = 1000000;
    if (_methods.empty())
        _methods = {"GET", "POST", "DELETE"};
    if (_host.empty())
        _host = "127.0.0.1";
}