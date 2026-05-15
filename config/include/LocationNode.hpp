#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ASTNode.hpp"

class LocationNode : public ASTNode {
    private:
        std::string _path;
        std::vector<std::unique_ptr<ASTNode> > _directives;
    public:
        LocationNode(std::string path): _path(path) {}
        ~LocationNode() = default;

        const std::string& getPath() const { return _path; }
        const std::vector<std::unique_ptr<ASTNode>>& getDirectives() const { return _directives; }
        void addDirective(std::unique_ptr<ASTNode> directive) noexcept;
};