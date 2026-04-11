#include "ServerNode.hpp"

void ServerNode::addDirective(std::unique_ptr<ASTNode> directive) noexcept
{
    _directives.push_back(std::move(directive));
}

void ServerNode::addLocation(std::unique_ptr<ASTNode> location) noexcept
{
    _locations.push_back(std::move(location));
}

