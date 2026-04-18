#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class ListenNode : public ASTNode
{
    private:
        int _port;
    public:
        ListenNode(int port) : _port(port) {}
        ~ListenNode() = default;
        const int& getPort() const { return _port; }
};