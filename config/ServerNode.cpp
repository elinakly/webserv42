#include "ServerNode.hpp"
#include "ListenNode.hpp"
#include "RootNode.hpp"
#include "ServerNameNode.hpp"
#include "MaxBodySizeNode.hpp"
#include "ErrorPageNode.hpp"
#include "AllowedMethodsNode.hpp"
#include "IndexNode.hpp"
#include "HostNode.hpp"


ServerNode::ServerNode() : _port(0), _max_body_size(0)
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

void ServerNode::resolve()
{
    for (size_t i = 0; i < _directives.size(); i++)
    {
        ASTNode* node = _directives[i].get();

        if (ListenNode* listen = dynamic_cast<ListenNode*>(node))
            _port = listen->getPort();
        else if (RootNode* root = dynamic_cast<RootNode*>(node))
            _root = root->getPath();
        else if (ServerNameNode* name = dynamic_cast<ServerNameNode*>(node))
            _server_name = name->getName();
        else if (MaxBodySizeNode* body = dynamic_cast<MaxBodySizeNode*>(node))
            _max_body_size = body->getSize();
        else if (ErrorPageNode* err = dynamic_cast<ErrorPageNode*>(node))
        {
            const std::map<int, std::string>& errs = err->getErrors();
            _errors.insert(errs.begin(), errs.end());
        }
        else if (AllowedMethodsNode* methods = dynamic_cast<AllowedMethodsNode*>(node))
            _methods = methods->getMethods();
        else if (IndexNode* index = dynamic_cast<IndexNode*>(node))
            _index_path = index->getPath();
        else if(HostNode* host = dynamic_cast<HostNode*> (node))
            _host = host->getHost();
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