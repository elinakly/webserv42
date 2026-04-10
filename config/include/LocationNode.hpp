#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class LocationNode : public ASTNode {
    private:
        std::string _path;
public:
    LocationNode(std::string path): _path(path) {}
    ~LocationNode() = default;

    std::string getPath() const { return _path; }
    void addDirective(ASTNode* directive);
};