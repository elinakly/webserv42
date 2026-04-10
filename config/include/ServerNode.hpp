#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class LocationNode;
class DirectiveNode;

class ServerNode : public ASTNode {
    private:
        std::vector<ASTNode*> _directives;
        std::vector<ASTNode*> _locations;
public:
    ServerNode() = default;
    ~ServerNode() = default;
    
    std::vector<ASTNode*> getDirectives() {return (_directives); };
    std::vector<ASTNode*> getLocations() {return (_locations); }

    void ServerNode::addDirective(ASTNode* directive)
    {
        _directives.push_back(directive);
    }

    void ServerNode::addLocation(ASTNode* location)
    {
        _locations.push_back(location);
    }
};
