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

class LocationNode : public ASTNode {
    private:
        std::string _path;
        bool _isRedir;
        std::string _newPath;

        std::vector<std::unique_ptr<ASTNode> > _directives;

        std::string _root;
        std::string _index_path;
        std::vector<std::string> _methods;

    public:
        LocationNode(std::string path): _path(path) , _isRedir(false){}
        ~LocationNode() = default;

        const std::string& getPath() const;
        const std::string& getNewPath() const;
        bool getIsRedir() const;
        const std::string& getRoot() const;
        const std::string& getIndexPath() const;
        const std::vector<std::string>& getMethods() const;
        
        void resolve(const ServerNode& serverNode);
        void setPath(const std::string& path);
        void setNewPath(const std::string& newPath);
        void setRedir(bool isRedir);
        
        void addDirective(std::unique_ptr<ASTNode> directive) noexcept;
};