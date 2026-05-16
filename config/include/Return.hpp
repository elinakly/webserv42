#pragma once

#include <string>
#include "ASTNode.hpp"

class Return : public ASTNode {
    private:
        std::string _newPath;
    public:
        Return(std::string newPath) : _newPath(newPath) {}
        ~Return() = default;

        const std::string& getNewPath() const { return _newPath; }
};