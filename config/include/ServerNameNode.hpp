#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class ServerNameNode : public ASTNode {
    private:
        std::string _name;
    public:
        ServerNameNode(std::string name) : _name(name) {}
        ~ServerNameNode() = default;

        std::string getName() const { return _name; }
    
};