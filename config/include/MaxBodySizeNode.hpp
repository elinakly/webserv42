#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

class MaxBodySizeNode : public ASTNode {
    private:
        int _size;
    public:
        MaxBodySizeNode(int size) : _size(size) {}
        ~MaxBodySizeNode() = default;

        int getSize() const { return _size; }
};