#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "ASTNode.hpp"

class LocationNode;
class DirectiveNode;

class ServerNode : public ASTNode {
    private:
        int _port;
        int _max_body_size;

        std::string _host;
        std::string _server_name;
        std::string _root;
        std::string _index_path;

        std::map<int, std::string> _errors;
        std::vector<std::string> _methods;

        std::vector<std::unique_ptr<ASTNode> > _directives;
        std::vector<std::unique_ptr<ASTNode> > _locations;
    public:
        ServerNode();
        ~ServerNode() = default;
        
        const std::vector<std::unique_ptr<ASTNode> >& getDirectives() const { return _directives; }
        const std::vector<std::unique_ptr<ASTNode> >& getLocations() const { return _locations; }
        
        const int& getPort() const { return _port; }
        const int& getMax_body_size() const {return _max_body_size; }

        const std::string& getHost() const { return _host; }
        const std::string& getServer_name() const { return _server_name; }
        const std::string& getRoot() const { return _root; }
        const std::string& getIndex_path() const {return _index_path; }

        const std::map<int, std::string>& getErrors() const { return _errors; }
        const std::vector<std::string>& getMethods() const {return _methods; }
        
        void addDirective(std::unique_ptr<ASTNode> directive) noexcept;
        void addLocation(std::unique_ptr<ASTNode> location) noexcept;

        void resolve();
};
