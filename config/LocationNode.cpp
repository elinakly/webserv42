#include "LocationNode.hpp"

const std::string& LocationNode::getPath() const
{ 
    return _path; 
}

const std::string& LocationNode::getNewPath() const
{ 
    return _newPath; 
}
bool LocationNode::getIsRedir() const 
{ 
    return _isRedir;
}

void LocationNode::resolve(const ServerNode &serverNode)
{
    _root = serverNode.getRoot();
    _index_path = serverNode.getIndex_path();
    _methods = serverNode.getMethods();

    for (size_t i = 0; i < _directives.size(); i++)
    {
        ASTNode* node = _directives[i].get();

        if (RootNode* root = dynamic_cast<RootNode*>(node))
            _root = root->getPath();
        else if (IndexNode* index = dynamic_cast<IndexNode*>(node))
            _index_path = index->getPath();
        else if (AllowedMethodsNode* methods = dynamic_cast<AllowedMethodsNode*>(node))
            _methods = methods->getMethods();
    }
}

void LocationNode::setPath(const std::string& path)
{ 
    _path = path;
}

void LocationNode::setNewPath(const std::string& newPath)
{ 
    _newPath = newPath;
}

void LocationNode::setRedir(bool isRedir)
{  
    _isRedir = isRedir;  
}

const std::string& LocationNode::getRoot() const
{ 
    return _root;
}
const std::string& LocationNode::getIndexPath() const
{ 
    return _index_path;
}
const std::vector<std::string>& LocationNode::getMethods() const
{ 
    return _methods;
}
