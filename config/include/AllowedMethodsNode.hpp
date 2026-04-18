#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class AllowedMethodsNode : public ASTNode {
    private:
        std::vector<std::string> _methods;
    public:
        AllowedMethodsNode(std::vector<std::string> methods) : _methods(methods) {}
        ~AllowedMethodsNode() = default;
        
        const std::vector<std::string>& getMethods() const{ return _methods; }
};