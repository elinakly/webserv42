#include "LocationNode.hpp"
#include "AutoIndexNode.hpp"
#include "iostream"
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

int LocationNode::getCode() const
{
    return _code;
}

bool LocationNode::getAutoIndex() const
{
    return(_autoIndex);
}

void LocationNode::setAutoIndex(bool value)
{
    _autoIndex = value;
}

void LocationNode::resolve(const ServerNode &serverNode)
{
    _root = serverNode.getRoot();
    _index_path = serverNode.getIndex_path();
    _methods = serverNode.getMethods();
    _cgi = serverNode.getCgi();

    for (size_t i = 0; i < _directives.size(); i++)
    {
        ASTNode* node = _directives[i].get();

        if (RootNode* root = dynamic_cast<RootNode*>(node))
            _root = root->getPath();
        else if (IndexNode* index = dynamic_cast<IndexNode*>(node))
            _index_path = index->getPath();
        else if (AllowedMethodsNode* methods = dynamic_cast<AllowedMethodsNode*>(node))
            _methods = methods->getMethods();
        else if (CgiNode* cgi = dynamic_cast<CgiNode*>(node))
        {
            if (cgi)
                _cgi = cgi->getCgi();
        }
        else if (AutoIndexNode *autoIndex = dynamic_cast<AutoIndexNode*>(node))
        {
            _autoIndex = autoIndex->getEnabled();
        }
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

void LocationNode::setCode(int code)
{
    _code = code;
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

const std::vector<std::string>& LocationNode::getAllowedMethods() const
{
    return _methods;
}
const CgiNode::cgi_map_type& LocationNode::getCgi() const
{
    return _cgi;
}