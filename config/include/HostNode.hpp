#pragma once

#include <string>
#include "ASTNode.hpp"

class HostNode : public ASTNode {
    private:
        std::string _host;
    public:
        HostNode(std::string host) : _host(host) {}
        ~HostNode() = default;
        
        const std::string& getHost() const{ return _host; }
    
};