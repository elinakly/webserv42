#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ASTNode.hpp"

class LocationNode;
class DirectiveNode;

class ServerNode : public ASTNode {
    private:
        std::vector<std::unique_ptr<ASTNode> > _directives;
        std::vector<std::unique_ptr<ASTNode> > _locations;
public:
    ServerNode() = default;
    ~ServerNode() = default;
    
    const std::vector<std::unique_ptr<ASTNode> >& getDirectives() const { return _directives; }
    const std::vector<std::unique_ptr<ASTNode> >& getLocations() const { return _locations; }

    void addDirective(std::unique_ptr<ASTNode> directive) noexcept;
    void addLocation(std::unique_ptr<ASTNode> location) noexcept;
};
