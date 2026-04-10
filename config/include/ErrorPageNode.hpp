#pragma once

#include <string>
#include <vector>
#include <map>
#include "ASTNode.hpp"

class ErrorPageNode : public ASTNode {
    private:
        std::map<int, std::string> errors;
    public:
        ErrorPageNode() = default;
        ~ErrorPageNode() = default;

        std::map<int, std::string> getErrors() const { return errors; }

        const void addErrors(int code, std::string& path);

        std::string getPath(int code);
};