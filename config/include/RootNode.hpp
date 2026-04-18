#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class RootNode : public ASTNode {
    private:
        std::string _path;
    public:
        RootNode(std::string path) : _path(path) {}
        ~RootNode() = default;
        
        const std::string& getPath() const{ return _path; }
    
};