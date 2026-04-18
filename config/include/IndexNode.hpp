#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class IndexNode : public ASTNode {
    private:
        std::string _path;
    public:
        IndexNode(std::string path) : _path(path) {}
        ~IndexNode() = default;
        
        const std::string& getPath() const{ return _path; }
    
};