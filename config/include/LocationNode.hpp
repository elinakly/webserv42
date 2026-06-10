#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ServerNode.hpp"
#include "ASTNode.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "AllowedMethodsNode.hpp"
#include "CgiNode.hpp"

class LocationNode : public ASTNode {
    private:
        std::string _path;
        bool _isRedir;
        std::string _newPath;
        int _code;

        std::vector<std::unique_ptr<ASTNode> > _directives;

        std::string _root;
        std::string _index_path;
        std::vector<std::string> _methods;
        CgiNode::cgi_map_type _cgi;

    public:
        LocationNode(std::string path): _path(path) , _isRedir(false), _cgi() {}
        ~LocationNode() = default;

        const std::string& getPath() const;
        const std::string& getNewPath() const;
        int getCode() const;
        bool getIsRedir() const;
        const std::string& getRoot() const;
        const std::string& getIndexPath() const;
        const std::vector<std::string>& getMethods() const;
        const std::vector<std::string>& getAllowedMethods() const;
        const CgiNode::cgi_map_type& getCgi() const;
        
        void resolve(const ServerNode& serverNode);
        void setPath(const std::string& path);
        void setCode(int code);
        void setNewPath(const std::string& newPath);
        void setRedir(bool isRedir);
        
        void addDirective(std::unique_ptr<ASTNode> directive) noexcept;
};