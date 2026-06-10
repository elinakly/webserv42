#include "CgiNode.hpp"

CgiNode::CgiNode() {}

CgiNode::~CgiNode() {}
//Adding a new writing to the map
void CgiNode::addCgi(const std::string& extension, const std::string& path) {
    _cgi[extension] = path;
}
//returning constant link to the map
const CgiNode::cgi_map_type& CgiNode::getCgi() const {
    return _cgi;
}

void CgiNode::resolve(const ASTNode& server) {
    (void)server;
}