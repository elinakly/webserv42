#pragma once

#include <string>
#include "ASTNode.hpp"

class Return : public ASTNode {
    private:
        std::string _newPath;
        int _code;
    public:
        Return(std::string newPath, int code) : _newPath(newPath), _code(code) {}
        ~Return() = default;

        const std::string& getNewPath() const { return _newPath; }
           int getCode() const { return _code; }
};