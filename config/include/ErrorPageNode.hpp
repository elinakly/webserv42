#pragma once

#include <string>
#include <vector>
#include <map>
#include "ASTNode.hpp"

class ErrorPageNode : public ASTNode {
    private:
        std::map<int, std::string> _errors;
    public:
        ErrorPageNode() = default;
        ~ErrorPageNode() = default;

        const std::map<int, std::string>& getErrors() const { return _errors; }

        void addErrors(int code, std::string& path);

        std::string getPath(int code) noexcept;
};