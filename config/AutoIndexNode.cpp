#include "AutoIndexNode.hpp"

AutoIndexNode::AutoIndexNode(bool enabled)
    : _enabled(enabled)
{
}

bool AutoIndexNode::getEnabled() const
{
    return _enabled;
}

void AutoIndexNode::resolve(const ASTNode& server)
{
    (void)server;
}